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
  : D_ifff(eqFct, doocsPropertyName), _i1Value(i1Value), _f1Value(f1Value), _f2Value(f2Value), _f3Value(f3Value),
    _updater(updater), _eqFct(eqFct), isWriteable(_i1Value->isWriteable() && _f1Value->isWriteable() &&
                                          _f2Value->isWriteable() && _f3Value->isWriteable()) {
    checkSourceConsistency();
    registerIfffSources();
    if(_i1Value->isReadOnly() && _f1Value->isReadOnly() && _f2Value->isReadOnly() && _f3Value->isReadOnly()) {
      this->set_ro_access();
    }
  }

  // Constructor without history
  DoocsIfff::DoocsIfff(std::string const& doocsPropertyName, EqFct* eqFct,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f2Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f3Value, DoocsUpdater& updater)
  : D_ifff(doocsPropertyName, eqFct), _i1Value(i1Value), _f1Value(f1Value), _f2Value(f2Value), _f3Value(f3Value),
    _updater(updater), _eqFct(eqFct), isWriteable(_i1Value->isWriteable() && _f1Value->isWriteable() &&
                                          _f2Value->isWriteable() && _f3Value->isWriteable()) {
    checkSourceConsistency();
    registerIfffSources();
    if(_i1Value->isReadOnly() && _f1Value->isReadOnly() && _f2Value->isReadOnly() && _f3Value->isReadOnly()) {
      this->set_ro_access();
    }
  }

  void DoocsIfff::checkSourceConsistency() {
    bool areAllSourcesWritable =
        (_i1Value->isWriteable() && _f1Value->isWriteable() && _f2Value->isWriteable() && _f3Value->isWriteable());
    bool areAllSourcesReadOnly =
        (_i1Value->isReadOnly() && _f1Value->isReadOnly() && _f2Value->isReadOnly() && _f3Value->isReadOnly());

    if(!areAllSourcesWritable && !areAllSourcesReadOnly) {
      ChimeraTK::logic_error("Doocs Adapter IFFF configuration Error: some IFFF sources are not writable");
    }
  }

  void DoocsIfff::registerIfffSources() {
    registerVariable(OneDRegisterAccessor<int>(_i1Value));
    registerVariable(OneDRegisterAccessor<float>(_f1Value));
    registerVariable(OneDRegisterAccessor<float>(_f2Value));
    registerVariable(OneDRegisterAccessor<float>(_f3Value));
  }

  void DoocsIfff::updateAppToDoocs(TransferElementID& elementId) {
    if(!_consistencyGroup.update(elementId)) {
      return;
    }

    bool storeInHistory = true;
    auto archiverStatus = ArchiveStatus::sts_ok;
    if(_i1Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f1Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f2Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f3Value->dataValidity() != ChimeraTK::DataValidity::ok) {
      if(this->d_error()) // data are alredy invalid, do not store in history
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

    doocs::Timestamp timestamp(_i1Value->getVersionNumber().getTime());

    // update global time stamp of DOOCS, but only if our time stamp is newer
    if(get_global_timestamp() < timestamp) {
      set_global_timestamp(timestamp);
    }

    // We should also checked if data should be stored (flag storeInHistory). Invalid data should NOT be stored except first invalid data point.
    // (https://github.com/ChimeraTK/ControlSystemAdapter-DoocsAdapter/issues/40)
    if(this->get_histPointer() && storeInHistory) {
      /*
      doocs::EventId eventId =
          (_macroPulseNumberSource) ? doocs::EventId(_macroPulseNumberSource->accessData(0)) : doocs::EventId(0);
      */

      /*FIXME: This set_and_archive does not support the timestamp yet (only sec and msec, and I guess m is milli?)*/
      /*FIXME: This set_and_archive does not support eventIDs yet */
      this->set_and_archive(&ifff, archiverStatus, 0, 0 /*msec*/);
    }
    else {
      this->set_value(&ifff);
    }

    auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
    auto seconds = sinceEpoch.seconds;
    auto microseconds = sinceEpoch.microseconds;

    this->set_tmstmp(seconds, microseconds);
    if(_macroPulseNumberSource) this->set_mpnum(_macroPulseNumberSource->accessData(0));

    // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    if(_publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
      dmsg_info info;
      memset(&info, 0, sizeof(info));
      info.sec = seconds;
      info.usec = microseconds;
      if(_macroPulseNumberSource != nullptr) {
        info.ident = _macroPulseNumberSource->accessData(0);
      }
      else {
        info.ident = 0;
      }
      auto ret = this->send(&info);
      if(ret) {
        std::cout << "ZeroMQ sending failed!!!" << std::endl;
      }
    }
  }

  void DoocsIfff::set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) {
    D_ifff::set(eqAdr, data1, data2, eqFct); // inherited functionality fill the local doocs buffer
    sendToApplication();

    // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    if(_publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
      auto timestamp = _i1Value->getVersionNumber().getTime();
      auto seconds = std::chrono::system_clock::to_time_t(timestamp);
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
          timestamp - std::chrono::system_clock::from_time_t(seconds))
                              .count();
      dmsg_info info;
      memset(&info, 0, sizeof(info));
      info.sec = seconds;
      info.usec = microseconds;
      if(_macroPulseNumberSource != nullptr) {
        info.ident = _macroPulseNumberSource->accessData(0);
      }
      else {
        info.ident = 0;
      }
      auto ret = this->send(&info);
      if(ret) {
        std::cout << "ZeroMQ sending failed!!!" << std::endl;
      }
    }
  }

  void DoocsIfff::auto_init(void) {
    D_ifff::auto_init(); // inherited functionality fill the local doocs buffer
    if(isWriteable) {
      sendToApplication();
    }
  }

  void DoocsIfff::sendToApplication() {
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
  }

  void DoocsIfff::setMacroPulseNumberSource(
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
    _macroPulseNumberSource = macroPulseNumberSource;
    if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
      registerVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(_macroPulseNumberSource));
    }
    else {
      // We don't need to match up anything with it when it changes, but we have to register this at least once
      // so the macropulse number will be included in the readAnyGroup in the updater if
      // <data_matching> is none everywhere
      _updater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct, []() {});
    }
  }

  void DoocsIfff::registerVariable(const ChimeraTK::TransferElementAbstractor& var) {
    if(var.isReadable()) {
      _updater.addVariable(var, _eqFct, std::bind(&DoocsIfff::updateAppToDoocs, this, var.getId()));
      _consistencyGroup.add(var);
    }
  }
} // namespace ChimeraTK
