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
      if(_errorSource.isValid() && _errorSource != transferElementId) return;
      // TODO maybe we need some kind of data consistency between error code and error string updates?
      // Current behaviour: set_error might be called more often than neccessary

      // Note: we already own the location lock by specification of the DoocsUpdater
      int statusCode = _statusScalar->accessData(0);
      int err_no = statusCodeMapping(statusCode);

      std::string err_str = _statusString ? _statusString->accessData(0) : "(null)";
      // set or clear error source id
      _errorSource = (err_no != 0) ? transferElementId : TransferElementID();
      _eqFct->set_error(err_no, err_str);
    }

    StatusHandler(EqFct* eqFct, boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> const& statusScalar,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> const& statusString,
        boost::shared_ptr<DoocsUpdater> const& updater)
    : _statusScalar(statusScalar), _statusString(statusString), _doocsUpdater(updater), _eqFct(eqFct) {
      if(!_statusScalar->isReadable()) throw ChimeraTK::logic_error(_statusScalar->getName() + " is not readable!");
      updater->addVariable(ChimeraTK::ScalarRegisterAccessor<int32_t>(_statusScalar), eqFct,
          std::bind(&StatusHandler::updateError, this, _statusScalar->getId()));
      if(_statusString) {
        if(!_statusString->isReadable()) throw ChimeraTK::logic_error(_statusString->getName() + " is not readable!");
        updater->addVariable(ChimeraTK::ScalarRegisterAccessor<std::string>(_statusString), eqFct,
            std::bind(&StatusHandler::updateError, this, _statusString->getId()));
      }
    }

    // StatusOutput and Device.status are both int32
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> _statusScalar;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> _statusString;

    //    void f() {
    //      // a mapping for StatusOutput values -> DOOCS error codes
    //      StatusAccessorBase::Status s;
    //      s == StatusAccessorBase::Status::OK;
    //      no_error;
    //      s == StatusAccessorBase::Status::FAULT;
    //      ill_function;
    //      s == StatusAccessorBase::Status::OFF;
    //      no_error; not_available;
    //      s == StatusAccessorBase::Status::WARNING;
    //      warning;
    //      // a mapping for DeviceModule.status values -> DOOCS error codes
    //      0;
    //      no_error;
    //      1;
    //      not_available;
    //    }

    // mapping function from Device.status/StatusOutput to DOOCS error codes
    // TODO details
    std::function<int(int)> statusCodeMapping{[](int x) { return x; }};

    boost::shared_ptr<DoocsUpdater> _doocsUpdater;
    EqFct* _eqFct;
    TransferElementID _errorSource;
  };

} // namespace ChimeraTK
