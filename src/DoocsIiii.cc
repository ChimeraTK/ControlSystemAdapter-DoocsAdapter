#include "DoocsIiii.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <doocs/EventId.h>
#include <functional>

namespace ChimeraTK {
  DoocsIiii::DoocsIiii(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i2Value,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i3Value,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i4Value, DoocsUpdater& updater)
  : D_iiii(eqFct, doocsPropertyName), _i1Value(i1Value), _i2Value(i2Value), _i3Value(i3Value), _i4Value(i4Value),
    _updater(updater), _eqFct(eqFct), isWriteable(_i1Value->isWriteable() && _i2Value->isWriteable() &&
                                          _i3Value->isWriteable() && _i4Value->isWriteable()) {
    checkSourceConsistency();
    registerIiiiSources();
    if(_i1Value->isReadOnly() && _i2Value->isReadOnly() && _i3Value->isReadOnly() && _i4Value->isReadOnly()) {
      this->set_ro_access();
    }
  }

  // Constructor without history
  DoocsIiii::DoocsIiii(std::string const& doocsPropertyName, EqFct* eqFct,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i2Value,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i3Value,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i4Value, DoocsUpdater& updater)
  : D_iiii(doocsPropertyName, eqFct), _i1Value(i1Value), _i2Value(i2Value), _i3Value(i3Value), _i4Value(i4Value),
    _updater(updater), _eqFct(eqFct), isWriteable(_i1Value->isWriteable() && _i2Value->isWriteable() &&
                                          _i3Value->isWriteable() && _i4Value->isWriteable()) {
    checkSourceConsistency();
    registerIiiiSources();
    if(_i1Value->isReadOnly() && _i2Value->isReadOnly() && _i3Value->isReadOnly() && _i4Value->isReadOnly()) {
      this->set_ro_access();
    }
  }

  void DoocsIiii::checkSourceConsistency() {
    bool areAllSourcesWritable =
        (_i1Value->isWriteable() && _i2Value->isWriteable() && _i3Value->isWriteable() && _i4Value->isWriteable());
    bool areAllSourcesReadOnly =
        (_i1Value->isReadOnly() && _i2Value->isReadOnly() && _i3Value->isReadOnly() && _i4Value->isReadOnly());

    if(!areAllSourcesWritable && !areAllSourcesReadOnly) {
      ChimeraTK::logic_error("Doocs Adapter IIII configuration Error: some IIII sources are not writable");
    }
  }

  void DoocsIiii::registerIiiiSources() {
    registerVariable(OneDRegisterAccessor<int>(_i1Value));
    registerVariable(OneDRegisterAccessor<int>(_i2Value));
    registerVariable(OneDRegisterAccessor<int>(_i3Value));
    registerVariable(OneDRegisterAccessor<int>(_i4Value));
  }

  void DoocsIiii::updateAppToDoocs(TransferElementID& elementId) {
    if(!_consistencyGroup.update(elementId)) {
      return;
    }

    if(_i1Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _i2Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _i3Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _i4Value->dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    IIII iiii;
    iiii.i1_data = _i1Value->accessData(0);
    iiii.i2_data = _i2Value->accessData(0);
    iiii.i3_data = _i3Value->accessData(0);
    iiii.i4_data = _i4Value->accessData(0);

    doocs::Timestamp timestamp(_i1Value->getVersionNumber().getTime());

    // update global time stamp of DOOCS, but only if our time stamp is newer
    if(get_global_timestamp() < timestamp) {
      set_global_timestamp(timestamp);
    }

    if(this->get_histPointer()) {
      /*
      doocs::EventId eventId =
          (_macroPulseNumberSource) ? doocs::EventId(_macroPulseNumberSource->accessData(0)) : doocs::EventId(0);
      */
      /*FIXME: The archiver also has a status code. Set it correctly.*/
      /*FIXME: This set_and_archive does not support the timestamp yet (only sec and msec, and I guess m is milli?)*/
      /*FIXME: This set_and_archive does not support eventIDs yet */
      this->set_and_archive(&iiii, ArchiveStatus::sts_ok, 0, 0 /*msec*/);
    }
    else {
      this->set_value(&iiii);
    }

    auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
    auto seconds = sinceEpoch.seconds;
    auto microseconds = sinceEpoch.microseconds;

    this->set_tmstmp(seconds, microseconds);
    if(_macroPulseNumberSource) this->set_mpnum(_macroPulseNumberSource->accessData(0));

    if(_publishZMQ) {
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
      this->send(&info);
    }
  }

  void DoocsIiii::set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) {
    D_iiii::set(eqAdr, data1, data2, eqFct); // inherited functionality fill the local doocs buffer
    sendToApplication();
  }

  void DoocsIiii::auto_init(void) {
    D_iiii::auto_init(); // inherited functionality fill the local doocs buffer
    if(isWriteable) {
      sendToApplication();
    }
  }

  void DoocsIiii::sendToApplication() {
    IIII* iiii = value();

    _i1Value->accessData(0) = iiii->i1_data;
    _i2Value->accessData(0) = iiii->i2_data;
    _i3Value->accessData(0) = iiii->i3_data;
    _i4Value->accessData(0) = iiii->i4_data;

    // write all with the same version number
    VersionNumber v = {};
    _i1Value->write(v);
    _i2Value->write(v);
    _i3Value->write(v);
    _i4Value->write(v);
  }

  void DoocsIiii::setMacroPulseNumberSource(
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
    // FIXME: Assuming macroPulseNumberSource is relavent only when all 4
    // components are readable; correct behavior later if this assumption
    // does not hold.
    bool isIiiiReadable =
        (_i1Value->isReadable() && _i2Value->isReadable() && _i3Value->isReadable() && _i4Value->isReadable());
    if(not isIiiiReadable) {
      return;
    }
    _macroPulseNumberSource = macroPulseNumberSource;
    if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
      registerVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(_macroPulseNumberSource));
    }
  }

  void DoocsIiii::registerVariable(const ChimeraTK::TransferElementAbstractor& var) {
    if(var.isReadable()) {
      _updater.addVariable(var, _eqFct, std::bind(&DoocsIiii::updateAppToDoocs, this, var.getId()));
      _consistencyGroup.add(var);
    }
  }
} // namespace ChimeraTK
