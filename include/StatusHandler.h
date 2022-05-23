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
   public:
    void updateError(TransferElementID transferElementId) {
      // TODO maybe we need some kind of data consistency between error code and error string updates?
      // Current behaviour: set_error might be called more often than neccessary

      bool foundVar = false;
      int err_no = 0;
      std::string err_str;
      for(auto const& entry : _varsToMonitor) {
        const VarToMonitor& var = entry.second;
        if(var.statusScalar->getId() == transferElementId ||
            (var.statusString && var.statusString->getId() == transferElementId)) {
          // Note: we already own the location lock by specification of the DoocsUpdater
          int statusCode = var.statusScalar->accessData(0);
          err_no = statusCodeMapping(statusCode, var.mappingType);
          err_str = var.statusString ? var.statusString->accessData(0) : "(null)";
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
              err_str = var.statusString ? var.statusString->accessData(0) : "(null)";
              _errorSource = var.statusScalar->getId();
            }
          }
        }
        if(v == VersionNumber(nullptr)) {
          // no more active errors
          _errorSource = TransferElementID();
        }
      }
      _eqFct->set_error(err_no, err_str);
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

    struct VarToMonitor {
      // StatusOutput and Device.status are both int32
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> statusScalar;
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> statusString;
      enum MappingType { MapStatusCode, MapDeviceError } mappingType;
    };
    // we use std::map as a set, TransferElementId of statusScalar is unique key
    std::map<TransferElementID, VarToMonitor> _varsToMonitor;

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

    boost::shared_ptr<DoocsUpdater> _doocsUpdater;
    EqFct* _eqFct;
    TransferElementID _errorSource;
  };

} // namespace ChimeraTK
