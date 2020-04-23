#include "DoocsIfff.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

namespace ChimeraTK {
  DoocsIfff::DoocsIfff(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f1Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f2Value,
      boost::shared_ptr<NDRegisterAccessor<float>> const& f3Value, DoocsUpdater& updater)
  : D_ifff(eqFct, doocsPropertyName), _i1Value(i1Value), _f1Value(f1Value), _f2Value(f2Value), _f3Value(f3Value) {
    auto registerSource = [&](const ChimeraTK::TransferElementAbstractor& var) {
      if(var.isReadable()) {
        updater.addVariable(var, eqFct, std::bind(&DoocsIfff::updateValues, this, var.getId()));
        _consistencyGroup.add(var);
      }
    };
    registerSource(OneDRegisterAccessor<int>(_i1Value));
    registerSource(OneDRegisterAccessor<float>(_f1Value));
    registerSource(OneDRegisterAccessor<float>(_f2Value));
    registerSource(OneDRegisterAccessor<float>(_f3Value));
  }

  void DoocsIfff::updateValues(TransferElementID& elementId) {
    if(_consistencyGroup.update(elementId)) {
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

      // we must not call set_and_archive if there is no history (otherwise it
      // will be activated), but we have to if it is there. -> Abstraction,
      // please!
      if(this->get_histPointer()) {
        // Set eventId
        //doocs::EventId eventId;
        //if(_macroPulseNumberSource) eventId = doocs::EventId(_macroPulseNumberSource->accessData(0));

        /*FIXME: The archiver also has a status code. Set it correctly.*/
        /*FIXME: This set_and_archive does not support the timestamp yet (only sec and msec, and I guess m is milli?)*/
        /*FIXME: This set_and_archive does not support eventIDs yet */
        this->set_and_archive(&ifff, ArchiveStatus::sts_ok, 0 /*sec*/, 0 /*msec*/);
      }
      else {
        this->set_value(&ifff);
      }
    }
  }
} // namespace ChimeraTK
