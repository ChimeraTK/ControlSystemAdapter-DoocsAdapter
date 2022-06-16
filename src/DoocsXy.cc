#include "DoocsXY.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

namespace ChimeraTK {
  DoocsXy::DoocsXy(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<float>> const& xValues,
      boost::shared_ptr<NDRegisterAccessor<float>> const& yValues, DoocsUpdater& updater)
  : D_xy(doocsPropertyName, xValues->getNumberOfSamples(), eqFct), _xValues(xValues), _yValues(yValues) {
    if(_xValues->isReadable()) {
      updater.addVariable(
          OneDRegisterAccessor<float>(xValues), eqFct, std::bind(&DoocsXy::updateDoocsBuffer, this, _xValues->getId()));
      _consistencyGroup.add(OneDRegisterAccessor<float>(xValues));
    }

    if(_yValues->isReadable()) {
      updater.addVariable(
          OneDRegisterAccessor<float>(yValues), eqFct, std::bind(&DoocsXy::updateDoocsBuffer, this, _yValues->getId()));
      _consistencyGroup.add(OneDRegisterAccessor<float>(yValues));
    }
  }

  void DoocsXy::updateDoocsBuffer(const TransferElementID& elementId) {
    if(_consistencyGroup.update(elementId)) {
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
    }
  }
} // namespace ChimeraTK
