#include "DoocsXY.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

namespace ChimeraTK {
  DoocsXy::DoocsXy(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<float>> const& xValues,
      boost::shared_ptr<NDRegisterAccessor<float>> const& yValues, DoocsUpdater& updater)
  : D_xy(doocsPropertyName, xValues->getNumberOfSamples(), eqFct), PropertyBase(eqFct, doocsPropertyName, updater),
    _xValues(xValues), _yValues(yValues) {
    init(xValues);
    init(yValues);
  }

  void DoocsXy::updateDoocsBuffer(const TransferElementID& elementId) {
    if(!updateConsistency(elementId)) {
      return;
    }

    if(_xValues->dataValidity() != ChimeraTK::DataValidity::ok ||
        _yValues->dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    for(int i = 0; i < max_length(); i++) {
      fill_xy(i, _xValues->accessData(i), _yValues->accessData(i));
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    if(_macroPulseNumberSource) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
