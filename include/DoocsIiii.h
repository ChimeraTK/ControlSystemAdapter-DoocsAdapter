// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DoocsAdapter.h"
#include <D_xy.h>

#include <ChimeraTK/NDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

class EqFct;

namespace ChimeraTK {
  class DoocsUpdater;

  class DoocsIiii : public D_iiii, public boost::noncopyable, public PropertyBase {
   public:
    /// Constructor with history enabled
    DoocsIiii(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<NDRegisterAccessor<int>> const& iiiiValue, DoocsUpdater& updater);

    /// Constructor without history
    DoocsIiii(std::string const& doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<NDRegisterAccessor<int>> const& iiiiValue, DoocsUpdater& updater);

    void set(EqAdr* eqAdr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqFct) override;
    void auto_init() override;

   protected:
    void updateDoocsBuffer(const TransferElementID& elementId) override;
    void sendToApplication(bool getLock);
    void registerIiiiSources();
    void checkSourceConsistency();

    boost::shared_ptr<NDRegisterAccessor<int>> _iiiiValue;

    bool _isWriteable;
  };
} // namespace ChimeraTK
