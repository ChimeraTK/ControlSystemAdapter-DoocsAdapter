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

  class DoocsIfff : public D_ifff, public boost::noncopyable, public PropertyBase {
   public:
    /// Constructor with history enabled
    DoocsIfff(EqFct* eqFct, std::string const& doocsPropertyName, boost::shared_ptr<NDRegisterAccessor<int>> i1Value,
        boost::shared_ptr<NDRegisterAccessor<float>> f1Value, boost::shared_ptr<NDRegisterAccessor<float>> f2Value,
        boost::shared_ptr<NDRegisterAccessor<float>> f3Value, DoocsUpdater& updater);

    /// Constructor without history
    DoocsIfff(std::string const& doocsPropertyName, EqFct* eqFct, boost::shared_ptr<NDRegisterAccessor<int>> i1Value,
        boost::shared_ptr<NDRegisterAccessor<float>> f1Value, boost::shared_ptr<NDRegisterAccessor<float>> f2Value,
        boost::shared_ptr<NDRegisterAccessor<float>> f3Value, DoocsUpdater& updater);

    void set(EqAdr* eqAdr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqFct) override;
    void auto_init() override;

   protected:
    void updateDoocsBuffer(const TransferElementID& elementId) override;
    void sendToApplication(bool getLock);
    void registerIfffSources();
    void checkSourceConsistency();

    boost::shared_ptr<NDRegisterAccessor<int>> _i1Value;
    boost::shared_ptr<NDRegisterAccessor<float>> _f1Value;
    boost::shared_ptr<NDRegisterAccessor<float>> _f2Value;
    boost::shared_ptr<NDRegisterAccessor<float>> _f3Value;

    bool _isWriteable{};
  };
} // namespace ChimeraTK
