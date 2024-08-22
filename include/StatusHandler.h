// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ChimeraTK/ControlSystemAdapter/StatusWithMessageReader.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class EqFct;
namespace ChimeraTK {

  class DoocsAdapter;
  class DoocsUpdater;

  /**
   * StatusHandler implements an update function for reporting errors to locations.
   * Input should be e.g. status of a DeviceModule or StatusAggregator.
   * Status codes are mapped to DOOCS and written via set_error(code, string)
   */
  class StatusHandler : public boost::noncopyable {
    boost::shared_ptr<DoocsUpdater> _doocsUpdater;
    EqFct* _eqFct;
    /// accessors to the status and string variables we want to monitor
    StatusWithMessageReader _varPair;

   public:
    void updateError(TransferElementID transferElementId);

    /// statusScalar and statusString are the variables to monitor. statusScalar is mandatory, statusString not.
    /// If both are set, updates must come in consistently
    StatusHandler(EqFct* eqFct, boost::shared_ptr<DoocsUpdater> updater,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int32_t>> const& statusScalar,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> const& statusString = nullptr);

    /// mapping function from Device.status/StatusOutput to DOOCS error codes
    static int statusCodeMapping(int x);
  };

} // namespace ChimeraTK
