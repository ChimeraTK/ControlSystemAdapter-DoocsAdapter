#pragma once

#include "DoocsUpdater.h"
#include "DoocsAdapter.h"
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <boost/shared_ptr.hpp>
#include <eq_fct.h>
#include <eq_errors.h>

// beware - StatusAccessor name clash with FAULT defined in ArchiveError.h included indirectly by d_fct.h
#undef FAULT
#include <ChimeraTK/ApplicationCore/StatusAccessor.h>

#include <ChimeraTK/ApplicationCore/StatusWithMessage.h>

namespace ChimeraTK {

  /**
 * StatusHandler implements an update function for reporting errors to locations.
 * Input should be e.g. status of a DeviceModule or StatusAggregator. 
 * Status codes are mapped to DOOCS and written via set_error(code, string)
 */
  class StatusHandler : public boost::noncopyable {
    boost::shared_ptr<DoocsUpdater> _doocsUpdater;
    EqFct* _eqFct;
    /// accessors to the status and string variables we want to monitor
    StatusWithMessageReader _varPair;

   public:
    void updateError(TransferElementID transferElementId) {
      if(!_varPair.update(transferElementId)) return;
      int statusCode = _varPair._status;
      int err_no = statusCodeMapping(statusCode);
      std::string err_str = _varPair.getMessage();

      // Note: we already own the location lock by specification of the DoocsUpdater
      // TODO discuss this - print "ok" instead of cleared error message
      if(err_no == no_error)
        _eqFct->set_error(err_no);
      else
        _eqFct->set_error(err_no, err_str);
    }

    /// statusScalar and statusString are the variables to monitor. statusScalar is mandatory, statusString not.
    /// If both are set, updates must come in consistently
    StatusHandler(EqFct* eqFct, boost::shared_ptr<DoocsUpdater> const& updater,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> const& statusScalar,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> const& statusString = nullptr)
    : _doocsUpdater(updater), _eqFct(eqFct), _varPair(ChimeraTK::ScalarRegisterAccessor<int32_t>(statusScalar)) {
      if(!statusScalar->isReadable()) throw ChimeraTK::logic_error(statusScalar->getName() + " is not readable!");

      _doocsUpdater->addVariable(
          _varPair._status, _eqFct, std::bind(&StatusHandler::updateError, this, statusScalar->getId()));
      if(statusString) {
        if(!statusString->isReadable()) throw ChimeraTK::logic_error(statusString->getName() + " is not readable!");
        _varPair.setMessageSource(ChimeraTK::ScalarRegisterAccessor<std::string>(statusString));

        _doocsUpdater->addVariable(
            _varPair._message, _eqFct, std::bind(&StatusHandler::updateError, this, statusString->getId()));
      }
    }

    // mapping function from Device.status/StatusOutput to DOOCS error codes
    int statusCodeMapping(int x) {
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
  };

} // namespace ChimeraTK
