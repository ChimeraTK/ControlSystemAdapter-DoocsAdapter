// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsIiii.h"

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

#include <doocs/EventId.h>

#include <functional>

namespace ChimeraTK {
  DoocsIiii::DoocsIiii(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<int>> const& iiiiValue, DoocsUpdater& updater)
  : D_iiii(eqFct, doocsPropertyName), PropertyBase(doocsPropertyName, updater), _iiiiValue(iiiiValue) {
    registerIiiiSources();
  }

  // Constructor without history
  DoocsIiii::DoocsIiii(std::string const& doocsPropertyName, EqFct* eqFct,
      boost::shared_ptr<NDRegisterAccessor<int>> const& iiiiValue, DoocsUpdater& updater)
  : D_iiii(doocsPropertyName, eqFct), PropertyBase(doocsPropertyName, updater), _iiiiValue(iiiiValue) {
    registerIiiiSources();
  }

  void DoocsIiii::registerIiiiSources() {
    if(_iiiiValue->getNumberOfSamples() != 4) {
      throw new ChimeraTK::logic_error(_iiiiValue->getName() + " does not have exactly 4 elements");
    }

    _isWriteable = _iiiiValue->isWriteable();
    if(_iiiiValue->isReadOnly()) {
      this->set_ro_access();
    }
    registerVariable(OneDRegisterAccessor<int>(_iiiiValue));
    _outputVarForVersionNum = _iiiiValue;
  }

  void DoocsIiii::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    bool storeInHistory = true;
    auto archiverStatus = ArchiveStatus::sts_ok;
    if(_iiiiValue->dataValidity() != ChimeraTK::DataValidity::ok) {
      if(this->d_error()) {
        // data are already invalid, do not store in history
        storeInHistory = false;
      }

      archiverStatus = ArchiveStatus::sts_err;
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    IIII iiii;
    iiii.i1_data = _iiiiValue->accessData(0);
    iiii.i2_data = _iiiiValue->accessData(1);
    iiii.i3_data = _iiiiValue->accessData(2);
    iiii.i4_data = _iiiiValue->accessData(3);

    doocs::Timestamp timestamp = correctDoocsTimestamp();
    if(_macroPulseNumberSource) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    // We should also checked if data should be stored (flag storeInHistory). Invalid data should NOT be stored except
    // first invalid data point. (https://github.com/ChimeraTK/ControlSystemAdapter-DoocsAdapter/issues/40)
    if(this->get_histPointer() && storeInHistory) {
      /*FIXME: This set_and_archive does not support the timestamp yet (only sec and msec, and I guess m is milli?)*/
      /*FIXME: This set_and_archive does not support eventIDs yet */
      this->set_and_archive(&iiii, archiverStatus, 0, 0 /*msec*/);
    }
    else {
      this->set_value(&iiii);
    }
    // Note, I guess above will in future look something like this:
    // doocs::EventId eventid(_macroPulseNumberSource->accessData(0));
    // this->set_value(ifff, timestamp, eventid, archiverStatus);
    // because the feature "do not store invalid data in history" is effectily lost
    // when set_value() now also stores to history.
    // However, currently it does _not_ yet.

    sendZMQ(timestamp);
  }

  void DoocsIiii::set(EqAdr* eqAdr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqFct) {
    D_iiii::set(eqAdr, data1, data2, eqFct); // inherited functionality fill the local doocs buffer
    if(_macroPulseNumberSource != nullptr) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    sendToApplication(true);

    sendZMQ(getTimestamp());
  }

  void DoocsIiii::auto_init() {
    doocsAdapter.beforeAutoInit();

    D_iiii::auto_init(); // inherited functionality fill the local doocs buffer
    if(_isWriteable) {
      sendToApplication(false);
    }
  }

  void DoocsIiii::sendToApplication(bool getLock) {
    IIII* iiii = value();

    _iiiiValue->accessData(0) = iiii->i1_data;
    _iiiiValue->accessData(1) = iiii->i2_data;
    _iiiiValue->accessData(2) = iiii->i3_data;
    _iiiiValue->accessData(3) = iiii->i4_data;

    // write all with the same version number
    auto timestamp = DoocsIiii::get_timestamp().to_time_point();
    VersionNumber v(timestamp);
    _iiiiValue->write(v);

    updateOthers(getLock);
  }

} // namespace ChimeraTK
