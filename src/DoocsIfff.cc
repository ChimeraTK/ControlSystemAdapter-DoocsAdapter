#include "DoocsIfff.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <doocs/EventId.h>

namespace ChimeraTK {
  DoocsIfff::DoocsIfff(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f2Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f3Value, DoocsUpdater& updater)
  : D_ifff(eqFct, doocsPropertyName), _i1Value(i1Value), _f1Value(f1Value), _f2Value(f2Value), _f3Value(f3Value),
    _updater(updater), _eqFct(eqFct) {
    auto registerSource = [&](const ChimeraTK::TransferElementAbstractor& var) {
      if(var.isReadable()) {
        updater.addVariable(var, eqFct, std::bind(&DoocsIfff::updateAppToDoocs, this, var.getId()));
        _consistencyGroup.add(var);
      }
    };

    // FIXME: What if not all 4 are readable? is it still valid to add
    // all to a consistency group then?
    registerSource(OneDRegisterAccessor<int>(_i1Value));
    registerSource(OneDRegisterAccessor<float>(_f1Value));
    registerSource(OneDRegisterAccessor<float>(_f2Value));
    registerSource(OneDRegisterAccessor<float>(_f3Value));

    // FIXME: get this from a constructor parameter isReadOnly so this can be turned off
    isWriteable = true;
    if(!_i1Value->isWriteable() || !_f1Value->isWriteable() || !_f2Value->isWriteable() || !_f3Value->isWriteable()) {
      isWriteable = false;
    }
  }

  void DoocsIfff::updateAppToDoocs(TransferElementID& elementId) {
    if(!_consistencyGroup.update(elementId)) {
      return;
    }

    if(_i1Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f1Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f2Value->dataValidity() != ChimeraTK::DataValidity::ok ||
        _f3Value->dataValidity() != ChimeraTK::DataValidity::ok) {
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
    auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
    auto seconds = sinceEpoch.seconds;
    auto microseconds = sinceEpoch.microseconds;
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
      this->set_and_archive(&ifff, ArchiveStatus::sts_ok, 0, 0 /*msec*/);
    }
    else {
      this->set_value(&ifff);
    }

    this->set_tmstmp(seconds, microseconds);
    if(_macroPulseNumberSource) this->set_mpnum(_macroPulseNumberSource->accessData(0));
  }

  void DoocsIfff::set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) {
    D_ifff::set(eqAdr, data1, data2, eqFct); // inherited functionality fill the local doocs buffer
    sendToApplication();
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
    // FIXME: Assuming macroPulseNumberSource is relavent only when all 4
    // components are readable; correct behavior later if this assumption
    // does not hold.
    bool isIfffReadable =
        (_i1Value->isReadable() && _f1Value->isReadable() && _f2Value->isReadable() && _f3Value->isReadable());
    if(not isIfffReadable) {
      return;
    }
    _macroPulseNumberSource = macroPulseNumberSource;
    if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
      registerVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(_macroPulseNumberSource));
    }
  }

  void DoocsIfff::registerVariable(const ChimeraTK::TransferElementAbstractor& var) {
    if(var.isReadable()) {
      _updater.addVariable(var, _eqFct, std::bind(&DoocsIfff::updateAppToDoocs, this, var.getId()));
      _consistencyGroup.add(var);
    }
  }
} // namespace ChimeraTK
