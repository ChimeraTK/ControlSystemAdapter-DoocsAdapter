// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

// name clash with DOOCS-#defined FAULT, so include this first and then #undef
#include <eq_errors.h>
#include <eq_fct.h>
#undef FAULT

#include "DoocsUpdater.h"
#include "StatusHandler.h"

#include <ChimeraTK/ControlSystemAdapter/StatusWithMessageReader.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

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
        return not_available;
      case StatusAccessorBase::Status::OFF:
        return device_offline;
      case StatusAccessorBase::Status::WARNING:
        return warning;
      default:
        return x;
    }
  }

} // namespace ChimeraTK
