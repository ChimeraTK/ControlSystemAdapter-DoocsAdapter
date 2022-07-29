#include "DoocsIfff.h"
#include "DoocsUpdater.h"
#include "DoocsAdapter.h"

#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <doocs/EventId.h>
#include <functional>

namespace ChimeraTK {
  DoocsIfff::DoocsIfff(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f2Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f3Value, DoocsUpdater& updater)
  : D_ifff(eqFct, doocsPropertyName), PropertyBase(eqFct, doocsPropertyName, updater), _i1Value(i1Value),
    _f1Value(f1Value), _f2Value(f2Value), _f3Value(f3Value) {
    checkSourceConsistency();
    registerIfffSources();
  }

  // Constructor without history
  DoocsIfff::DoocsIfff(std::string const& doocsPropertyName, EqFct* eqFct,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f2Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f3Value, DoocsUpdater& updater)
  : D_ifff(doocsPropertyName, eqFct), PropertyBase(eqFct, doocsPropertyName, updater), _i1Value(i1Value),
    _f1Value(f1Value), _f2Value(f2Value), _f3Value(f3Value) {
    checkSourceConsistency();
    registerIfffSources();
  }

  void DoocsIfff::checkSourceConsistency() {
    bool areAllSourcesWritable =
        (_i1Value->isWriteable() && _f1Value->isWriteable() && _f2Value->isWriteable() && _f3Value->isWriteable());
    _isWriteable = areAllSourcesWritable;
    bool areAllSourcesReadOnly =
        (_i1Value->isReadOnly() && _f1Value->isReadOnly() && _f2Value->isReadOnly() && _f3Value->isReadOnly());

    if(!areAllSourcesWritable && !areAllSourcesReadOnly) {
      ChimeraTK::logic_error("Doocs Adapter IFFF configuration Error: some IFFF sources are not writable");
    }
    if(areAllSourcesReadOnly) {
      this->set_ro_access();
    }
  }

  void DoocsIfff::registerIfffSources() {
    registerVariable(OneDRegisterAccessor<int>(_i1Value));
    registerVariable(OneDRegisterAccessor<float>(_f1Value));
    registerVariable(OneDRegisterAccessor<float>(_f2Value));
    registerVariable(OneDRegisterAccessor<float>(_f3Value));
    _mainOutputVar = _i1Value;
  }

  void DoocsIfff::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    bool storeInHistory = true;
    auto archiverStatus = ArchiveStatus::sts_ok;
    if(_i1Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f1Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f2Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f3Value->dataValidity() != ChimeraTK::DataValidity::ok) {
      if(this->d_error()) // data are already invalid, do not store in history
        storeInHistory = false;

      archiverStatus = ArchiveStatus::sts_err;
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    IFFF ifff;
    ifff.i1_data = _i1Value->accessData(0);
    ifff.f1_data = _f1Value->accessData(0);
    ifff.f2_data = _f2Value->accessData(0);
    ifff.f3_data = _f3Value->accessData(0);

    doocs::Timestamp timestamp = correctDoocsTimestamp();
    if(_macroPulseNumberSource) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    // We should also checked if data should be stored (flag storeInHistory). Invalid data should NOT be stored except first invalid data point.
    // (https://github.com/ChimeraTK/ControlSystemAdapter-DoocsAdapter/issues/40)
    if(this->get_histPointer() && storeInHistory) {
      /*FIXME: This set_and_archive does not support the timestamp yet (only sec and msec, and I guess m is milli?)*/
      /*FIXME: This set_and_archive does not support eventIDs yet */
      this->set_and_archive(&ifff, archiverStatus, 0, 0 /*msec*/);
    }
    else {
      this->set_value(&ifff);
    }
    // Note, I guess above will in future look something like this:
    //doocs::EventId eventid(_macroPulseNumberSource->accessData(0));
    //this->set_value(ifff, timestamp, eventid, archiverStatus);
    // because the feature "do not store invalid data in history" is effectily lost
    // when set_value() now also stores to history.
    // However, currently it does _not_ yet.

    sendZMQ(timestamp);
  }

  void DoocsIfff::set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) {
    D_ifff::set(eqAdr, data1, data2, eqFct); // inherited functionality fill the local doocs buffer
    if(_macroPulseNumberSource != nullptr) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    sendToApplication(true);

    sendZMQ(getTimestamp());
  }

  void DoocsIfff::auto_init(void) {
    doocsAdapter.before_auto_init();

    D_ifff::auto_init(); // inherited functionality fill the local doocs buffer
    if(_isWriteable) {
      sendToApplication(false);
    }
  }

  void DoocsIfff::sendToApplication(bool getLock) {
    IFFF* ifff = value();

    _i1Value->accessData(0) = ifff->i1_data;
    _f1Value->accessData(0) = ifff->f1_data;
    _f2Value->accessData(0) = ifff->f2_data;
    _f3Value->accessData(0) = ifff->f3_data;

    // write all with the same version number
    VersionNumber v = {};
    _i1Value->write(v);
    _f1Value->write(v);
    _f2Value->write(v);
    _f3Value->write(v);

    updateOthers(getLock);
  }

} // namespace ChimeraTK
