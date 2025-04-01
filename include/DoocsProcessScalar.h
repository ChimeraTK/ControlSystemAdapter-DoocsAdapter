// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/ScalarRegisterAccessor.h>

#include <boost/shared_ptr.hpp>

#include <d_fct.h>
#include <eq_fct.h>

#include <chrono>
#include <string>

namespace ChimeraTK {

  /** The DoocsProcessScalar has three template parameters:
   *  \li \c T, The primitive value type of the ChimeraTK process variable
   *  \li \c DOOCS_T, The Doocs type which is used
   */
  template<typename T, typename DOOCS_T>
  class DoocsProcessScalar : public DOOCS_T, public boost::noncopyable, public PropertyBase {
   public:
    /// constructor with history: EqFtc first
    DoocsProcessScalar(EqFct* eqFct, std::string doocsPropertyName,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater);
    /// constructor without history: name first
    DoocsProcessScalar(std::string doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater);

    /**
     * Override the Doocs set method which is triggered by the RPC calls.
     */
    void set(EqAdr* adr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqfct) override;

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init() override;

   protected:
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> _processScalar;
  };

  /*******************************************************************************/

  template<typename T, typename DOOCS_T>
  DoocsProcessScalar<T, DOOCS_T>::DoocsProcessScalar(EqFct* eqFct, std::string doocsPropertyName,
      boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
  : DOOCS_T(eqFct, doocsPropertyName), PropertyBase(doocsPropertyName, updater), _processScalar(processScalar) {
    setupOutputVar(processScalar);
  }

  template<typename T, typename DOOCS_T>
  DoocsProcessScalar<T, DOOCS_T>::DoocsProcessScalar(std::string doocsPropertyName, EqFct* eqFct,
      boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
  : DOOCS_T(doocsPropertyName, eqFct), PropertyBase(doocsPropertyName, updater), _processScalar(processScalar) {
    setupOutputVar(processScalar);
  }

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::set(EqAdr* adr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqfct) {
    // only assign the value if the variable is writeable
    // Otherwise the content displayed by Doocs and the value in the application
    // are inconsistent
    if(!_processScalar->isWriteable()) {
      throw ChimeraTK::logic_error("Trying to write to a non-writable variable");
    }
    // note, current doocs implementation does not take error code, timestamp or event id from input data
    // it takes over data only if no error, and replaces timestamp and event id by global ones
    DOOCS_T::set(adr, data1, data2, eqfct);
    if(_macroPulseNumberSource != nullptr) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    // let the DOOCS_T set function do all the dirty work and use the
    // get_value function afterwards to get the already assigned value
    _processScalar->accessData(0) = this->value();
    auto timestamp = DOOCS_T::get_timestamp().to_time_point();
    _processScalar->write(VersionNumber(timestamp));

    updateOthers(true);

    sendZMQ(getTimestamp());
  }

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::auto_init() {
    doocsAdapter.beforeAutoInit();

    DOOCS_T::auto_init();
    // send the current value to the device
    // property is writeable OR the target accessor is writable and the only one connected to this property
    // The second case is to have bi-directional variables that are used to persist settings into the config file
    // and need that value back on start-up but are not supposed to be written by the control system and mapped
    // read-only in the configuration file.
    if(this->get_access() == 1 || (_processScalar->isWriteable() && otherPropertiesToUpdate.empty())) {
      _processScalar->accessData(0) = DOOCS_T::value();
      _processScalar->write();
      // set DOOCS time stamp, workaround for DOOCS bug (get() always gives current time stamp if no timestamp is set,
      // which breaks consistency check in ZeroMQ subscriptions after the 4 minutes timeout)
      DOOCS_T::set_stamp();

      updateOthers(false);
    }
  }

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    // Note: we already own the location lock by specification of the
    // DoocsUpdater
    auto data = _processScalar->accessData(0);

    auto archiverStatus = ArchiveStatus::sts_ok;
    if(_processScalar->dataValidity() != ChimeraTK::DataValidity::ok) {
      archiverStatus = ArchiveStatus::sts_err;
      // set data invalid in DOOCS for current data
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    doocs::EventId eventId;
    if(_macroPulseNumberSource) {
      eventId = doocs::EventId(_macroPulseNumberSource->accessData(0));
    }
    this->set_value(data, timestamp, eventId, archiverStatus);
    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
