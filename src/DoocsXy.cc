// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsXY.h"

#include "DoocsUpdater.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

namespace ChimeraTK {
  DoocsXy::DoocsXy(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<float>> const& xValues,
      boost::shared_ptr<NDRegisterAccessor<float>> const& yValues, DoocsUpdater& updater,
      DataConsistencyGroup::MatchingMode matchingMode)
  : D_xy(doocsPropertyName, xValues->getNumberOfSamples(), eqFct),
    PropertyBase(doocsPropertyName, updater, matchingMode), _xValues(xValues), _yValues(yValues) {
    setupOutputVar(_xValues);
    setupOutputVar(_yValues);
  }

  void DoocsXy::updateDoocsBuffer(const TransferElementID& elementId) {
    if(!updateConsistency(elementId)) {
      return;
    }

    if(_xValues.dataValidity() != ChimeraTK::DataValidity::ok ||
        _yValues.dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    for(int i = 0; i < max_length(); i++) {
      fill_xy(i, _xValues[i], _yValues[i]);
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    if(_macroPulseNumberSource.isInitialised()) {
      this->set_mpnum(_macroPulseNumberSource);
    }
    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
