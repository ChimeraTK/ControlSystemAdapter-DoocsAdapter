// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "PropertyBase.h"

#include <ChimeraTK/NDRegisterAccessor.h>

#include <D_xy.h>

class EqFct;

namespace ChimeraTK {
  class DoocsUpdater;

  class DoocsXy : public D_xy, public boost::noncopyable, public PropertyBase {
   public:
    DoocsXy(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<NDRegisterAccessor<float>> const& xValues,
        boost::shared_ptr<NDRegisterAccessor<float>> const& yValues, DoocsUpdater& updater,
        DataConsistencyGroup::MatchingMode matchingMode);

    void auto_init() override;

   protected:
    void updateDoocsBuffer(const TransferElementID& elementId) override;
    /// Apply the stored description and axis configuration via the D_xy
    /// set_descr_value() and set_plot_{x,y}_value() APIs.
    void applyDescriptionUnits(D_hist* hist) override;

    OneDRegisterAccessor<float> _xValues;
    OneDRegisterAccessor<float> _yValues;
  };
} // namespace ChimeraTK
