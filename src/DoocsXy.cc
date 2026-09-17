// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsXY.h"

#include "DoocsUpdater.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <ctime>

namespace ChimeraTK {

  /********************************************************************************************************************/

  DoocsXy::DoocsXy(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<NDRegisterAccessor<float>> const& xValues,
      boost::shared_ptr<NDRegisterAccessor<float>> const& yValues, DoocsUpdater& updater,
      DataConsistencyGroup::MatchingMode matchingMode)
  : D_xy(doocsPropertyName, xValues->getNumberOfSamples(), eqFct),
    PropertyBase(doocsPropertyName, updater, matchingMode), _xValues(xValues), _yValues(yValues) {
    setupOutputVar(_xValues);
    setupOutputVar(_yValues);
  }

  /********************************************************************************************************************/

  void DoocsXy::auto_init() {
    D_xy::auto_init();

    applyDescriptionUnits(nullptr);
  }

  /********************************************************************************************************************/

  void DoocsXy::applyDescriptionUnits(D_hist* hist) {
    (void)hist;
    if(_hasDescription) {
      set_descr_value(_description);
    }
    for(const auto& [key, a] : _axes) {
      char* label = const_cast<char*>(a.label.c_str());
      if(key == 'x') {
        this->set_plot_x_value(a.logarithmic, a.start, a.stop, std::time(nullptr), label);
      }
      else {
        this->set_plot_y_value(a.logarithmic, a.start, a.stop, std::time(nullptr), label);
      }
    }
  }

  /********************************************************************************************************************/

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
    sendAsync(timestamp);
  }

  /********************************************************************************************************************/

} // namespace ChimeraTK
