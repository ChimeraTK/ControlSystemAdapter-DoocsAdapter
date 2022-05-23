#pragma once

#include "DoocsUpdater.h"
#include "DoocsAdapter.h"
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <boost/shared_ptr.hpp>
#include <chrono>
#include <d_fct.h>
#include <eq_fct.h>
#include <eq_errors.h>
#include <string>

// beware - StatusAccessor name clash with FAULT defined in ArchiveError.h included indirectly by d_fct.h
#undef FAULT
#include <ChimeraTK/ApplicationCore/StatusAccessor.h>

/*
 * 
 * Concept Idea:
 * We need a special update function for reporting errors to locations.
 * It does not make sense to do this via evaluation of all mapped properties,
 * since most of the time, the error would be due to 
 * - device failure
 * - status aggregator has bad result
 * so the outputs of these modules (DeviceModule, StatusAggregator) 
 * should be mapped to DOOCS set_error(code, string) function
 * but we still need to specificy the affected locations somehow, in Doocs Adapter config.
 * To be discussed: behavior when multiple stati are collected.
 * For the moment, choose a very simple implementation:
 * Id of causing transfer element is stored. Error must be cleared by same id before a different source can set it.
 */

namespace ChimeraTK {

  class StatusHandler : public boost::noncopyable {
    struct VarToMonitor {
      // StatusOutput and Device.status are both int32
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> statusScalar;
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> statusString;
      enum MappingType { MapStatusCode, MapDeviceError } mappingType;
    };
    // we use std::map as a set, TransferElementId of statusScalar is unique key
    std::map<TransferElementID, VarToMonitor> _varsToMonitor;

    boost::shared_ptr<DoocsUpdater> _doocsUpdater;
    EqFct* _eqFct;
    TransferElementID _errorSource;

   public:
    void updateError(TransferElementID transferElementId) {
      // TODO do we need some kind of data consistency between error code and error string updates?
      // Current behaviour: set_error is called more often than neccessary
      // this leads to log which is a bit confusing.
      // Problem - sometimes device error message is updated without status change/update, so we don't know whether we should wait on consistent state with status

      bool foundVar = false;
      int err_no = 0;
      std::string err_str;
      for(auto const& entry : _varsToMonitor) {
        const VarToMonitor& var = entry.second;
        if((var.statusScalar && var.statusScalar->getId() == transferElementId) ||
            (var.statusString && var.statusString->getId() == transferElementId)) {
          // TODO discuss whether this approach is fine - skip trigger event from statusScalar if statusString is present
          //if(var.statusString && var.statusScalar->getId() == transferElementId) return;

          int statusCode;
          if(var.statusScalar)
            statusCode = var.statusScalar->accessData(0);
          else
            // TODO discuss - does it make sense to leave out statusScalar input and deduce statusCode from string? It would simplify things, but works only if error the only code used.
            statusCode = var.statusString->accessData(0) != "";
          err_no = statusCodeMapping(statusCode, var.mappingType);
          err_str = errorStringForVar(var);
          foundVar = true;
        }
      }
      assert(foundVar); // we have found updated values

      if(err_no == 0) {
        // new or re-triggered error
        _errorSource = transferElementId;
      }
      else {
        // clear only if no other errors
        // find most recent occurred error and take over code and string
        VersionNumber v(nullptr);
        for(auto const& entry : _varsToMonitor) {
          const VarToMonitor& var = entry.second;
          auto vn = var.statusScalar->getVersionNumber();
          if(vn > v) {
            int statusCode = var.statusScalar->accessData(0);
            int errMapped = statusCodeMapping(statusCode, var.mappingType);
            if(errMapped != 0) {
              v = vn;
              err_no = errMapped;
              err_str = errorStringForVar(var);
              _errorSource = var.statusScalar->getId();
            }
          }
        }
        if(v == VersionNumber(nullptr)) {
          // no more active errors
          _errorSource = TransferElementID();
        }
      }
      // Note: we already own the location lock by specification of the DoocsUpdater
      // TODO discuss - maybe printing "ok" instead of cleared error makes sense.
      if(err_no == no_error)
        _eqFct->set_error(err_no);
      else
        _eqFct->set_error(err_no, err_str);
    }

    std::string errorStringForVar(const VarToMonitor& var) {
      if(var.mappingType == var.MapDeviceError) {
        assert(var.statusString);
        return var.statusString->accessData(0);
      }
      else {
        int statusCode = var.statusScalar->accessData(0);
        auto err = StatusAccessorBase::Status(statusCode);
        std::string statusString;
        switch(err) {
          case StatusAccessorBase::Status::OK:
            statusString = "OK";
            break;
          case StatusAccessorBase::Status::FAULT:
            statusString = "FAULT";
            break;
          case StatusAccessorBase::Status::OFF:
            statusString = "OFF";
            break;
          case StatusAccessorBase::Status::WARNING:
            statusString = "WARNING";
            break;
          default:
            assert(false);
        }
        return var.statusScalar->getName() + " switched to " + statusString;
      }
    }

    StatusHandler(EqFct* eqFct, boost::shared_ptr<DoocsUpdater> const& updater)
    : _doocsUpdater(updater), _eqFct(eqFct) {}

    void addVariable(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> const& statusScalar,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> const& statusString) {
      {
        if(!statusScalar->isReadable()) throw ChimeraTK::logic_error(statusScalar->getName() + " is not readable!");
        TransferElementID transferElementId = statusScalar->getId();
        VarToMonitor& var = _varsToMonitor[transferElementId];
        var.statusScalar = statusScalar;

        _doocsUpdater->addVariable(ChimeraTK::ScalarRegisterAccessor<int32_t>(statusScalar), _eqFct,
            std::bind(&StatusHandler::updateError, this, statusScalar->getId()));
        if(statusString) {
          if(!statusString->isReadable()) throw ChimeraTK::logic_error(statusString->getName() + " is not readable!");
          var.statusString = statusString;

          _doocsUpdater->addVariable(ChimeraTK::ScalarRegisterAccessor<std::string>(statusString), _eqFct,
              std::bind(&StatusHandler::updateError, this, statusString->getId()));
        }
        // here we just assume devices have status strings and StatusOutputs not
        if(statusString)
          var.mappingType = VarToMonitor::MapDeviceError;
        else
          var.mappingType = VarToMonitor::MapStatusCode;
      }
    }

    // mapping function from Device.status/StatusOutput to DOOCS error codes
    int statusCodeMapping(int x, VarToMonitor::MappingType mappingType) {
      if(mappingType == VarToMonitor::MapStatusCode) {
        auto err = StatusAccessorBase::Status(x);
        // a mapping for StatusOutput values -> DOOCS error codes
        switch(err) {
          case StatusAccessorBase::Status::OK:
            return no_error;
          case StatusAccessorBase::Status::FAULT:
            //return ill_function;
            return not_available; // better compatibility with mapping below
          case StatusAccessorBase::Status::OFF:
            return no_error; // or  not_available ?
          case StatusAccessorBase::Status::WARNING:
            return warning;
          default:
            return x;
        }
      }
      else {
        // a mapping for DeviceModule.status values -> DOOCS error codes
        if(x == 0)
          return no_error;
        else
          return not_available;
      }
    }
  };

} // namespace ChimeraTK
