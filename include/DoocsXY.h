// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DoocsAdapter.h"

#include <ChimeraTK/NDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

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

   protected:
    void updateDoocsBuffer(const TransferElementID& elementId) override;

    boost::shared_ptr<NDRegisterAccessor<float>> _xValues;
    boost::shared_ptr<NDRegisterAccessor<float>> _yValues;
  };
} // namespace ChimeraTK
