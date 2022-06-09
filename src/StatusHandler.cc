
// name clash of DOOCS-defined error code, so include this first and then remove #define FAULT
#include <eq_errors.h>
#include <eq_fct.h>
#undef FAULT

#include "StatusHandler.h"

#include "DoocsUpdater.h"
#include "DoocsAdapter.h"
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <ChimeraTK/ControlSystemAdapter/StatusWithMessageReaderBase.h>
#include <boost/shared_ptr.hpp>

namespace ChimeraTK {

  void StatusHandler::updateError(TransferElementID transferElementId) {
    if(!_varPair.update(transferElementId)) return;
    int statusCode = _varPair._status;
    int err_no = statusCodeMapping(statusCode);
    std::string err_str = _varPair.getMessage();

    // Note: we already own the location lock by specification of the DoocsUpdater
    if(err_no == no_error)
      _eqFct->set_error(err_no);
    else
      _eqFct->set_error(err_no, err_str);
  }

  StatusHandler::StatusHandler(EqFct* eqFct, boost::shared_ptr<DoocsUpdater> const& updater,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> const& statusScalar,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> const& statusString)
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

  int StatusHandler::statusCodeMapping(int x) {
    auto err = StatusAccessorBase::Status(x);
    // a mapping for StatusOutput values -> DOOCS error codes
    switch(err) {
      case StatusAccessorBase::Status::OK:
        return no_error;
      case StatusAccessorBase::Status::FAULT:
        //return ill_function;
        return not_available; // better compatibility with mapping below
      case StatusAccessorBase::Status::OFF:
        return device_offline; // no_error; // or  not_available ?
      case StatusAccessorBase::Status::WARNING:
        return warning;
      default:
        return x;
    }
  }

} // namespace ChimeraTK
