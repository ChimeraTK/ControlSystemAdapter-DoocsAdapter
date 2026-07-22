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
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater,
        DataConsistencyGroup::MatchingMode matchingMode);
    /// constructor without history: name first
    DoocsProcessScalar(std::string doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater,
        DataConsistencyGroup::MatchingMode matchingMode);

    /**
     * Override the Doocs set method which is triggered by the RPC calls.
     */
    void set(EqAdr* adr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqfct) override;

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init() override;

    /** Set the EGU axis configuration for this scalar.
     *  Applied in auto_init() after the DOOCS framework loads persisted values from the .conf file.
     *  If set, the .EGU sub-property is made read-only, preventing runtime modification. */
    void setAxisConfig(int logarithmic, float start, float stop, const std::string& label);

   protected:
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    /// Apply EGU axis configuration from XML after auto_init().
    /// If the EGU is defined in XML, force-sets the value via set_plot_value() and
    /// makes the .EGU sub-property read-only. On mismatch with the .conf value,
    /// logs a warning and overwrites with the XML value.
    void applyAxisConfig();

    ScalarRegisterAccessor<T> _processScalar;

    /// EGU axis configuration from the XML file, applied after auto_init()
    struct AxisConfig {
      std::string label;
      int logarithmic{};
      float start{};
      float stop{};
    };
    AxisConfig _axisConfig;
    bool _axisConfigSet{false};
  };

  /********************************************************************************************************************/

  template<typename T, typename DOOCS_T>
  DoocsProcessScalar<T, DOOCS_T>::DoocsProcessScalar(EqFct* eqFct, std::string doocsPropertyName,
      boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater,
      DataConsistencyGroup::MatchingMode matchingMode)
  : DOOCS_T(eqFct, doocsPropertyName), PropertyBase(doocsPropertyName, updater, matchingMode),
    _processScalar(processScalar) {
    setupOutputVar(_processScalar);
  }

  /********************************************************************************************************************/

  template<typename T, typename DOOCS_T>
  DoocsProcessScalar<T, DOOCS_T>::DoocsProcessScalar(std::string doocsPropertyName, EqFct* eqFct,
      boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater,
      DataConsistencyGroup::MatchingMode matchingMode)
  : DOOCS_T(doocsPropertyName, eqFct), PropertyBase(doocsPropertyName, updater, matchingMode),
    _processScalar(processScalar) {
    setupOutputVar(_processScalar);
  }

  /********************************************************************************************************************/

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::set(EqAdr* adr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqfct) {
    // only assign the value if the variable is writeable
    // Otherwise the content displayed by Doocs and the value in the application
    // are inconsistent
    if(!_processScalar.isWriteable()) {
      throw ChimeraTK::logic_error("Trying to write to a non-writable variable");
    }
    // note, current doocs implementation does not take error code, timestamp or event id from input data
    // it takes over data only if no error, and replaces timestamp and event id by global ones
    DOOCS_T::set(adr, data1, data2, eqfct);
    if(_macroPulseNumberSource.isInitialised()) {
      this->set_mpnum(_macroPulseNumberSource);
    }
    // let the DOOCS_T set function do all the dirty work and use the
    // get_value function afterwards to get the already assigned value
    _processScalar = this->value();
    auto timestamp = DOOCS_T::get_timestamp().to_time_point();
    _processScalar.write(VersionNumber(timestamp));

    updateOthers(true);

    sendZMQ(getTimestamp());
  }

  /********************************************************************************************************************/

  /********************************************************************************************************************/

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::setAxisConfig(
      int logarithmic, float start, float stop, const std::string& label) {
    _axisConfig.logarithmic = logarithmic;
    _axisConfig.start = start;
    _axisConfig.stop = stop;
    _axisConfig.label = label;
    _axisConfigSet = true;
  }

  /********************************************************************************************************************/

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::applyAxisConfig() {
    if(!_axisConfigSet) {
      return;
    }
    if constexpr(std::is_base_of_v<D_text, DOOCS_T>) {
      return;
    }
    else {
      auto* hist = this->get_histPointer();
      if(hist == nullptr) {
        return;
      }
      // Read the EGU loaded from the .conf file (may be empty on first start)
      std::string confEgu = hist->egu();
      if(!confEgu.empty() && confEgu == _axisConfig.label) {
        // .conf EGU matches XML → nothing to do, but make .EGU read-only to prevent
        // runtime modification by clients.
        auto* eguProp = getEqFct()->find_property(this->basename() + ".EGU");
        if(eguProp != nullptr) {
          eguProp->set_ro_access();
        }
        return;
      }
      if(!confEgu.empty() && confEgu != _axisConfig.label) {
        // .conf EGU differs from XML → this indicates the .conf was manually modified
        // or the XML was changed after the server was first started.
        // Log a warning, force-set the XML value, and make .EGU read-only.
        std::cerr << "WARNING: " << this->basename() << ": .conf EGU (\"" << confEgu << "\") differs from XML EGU (\""
                  << _axisConfig.label << "\"). Overwriting with XML value." << std::endl;
        // TODO: Uncomment to throw an error instead of silently overwriting:
        // throw ChimeraTK::logic_error(
        //     this->basename() + ": .conf EGU (\"" + confEgu + "\") differs from XML EGU (\"" +
        //     _axisConfig.label + "\"). Cannot overwrite read-only XML unit.");
      }
      // Force-set the XML value (bypasses D_hist::egu()'s "set only if empty" guard)
      hist->set_plot_value(_axisConfig.logarithmic, _axisConfig.start, _axisConfig.stop,
          doocs::Timestamp::now().to_time_t(), _axisConfig.label.c_str());
      // Make .EGU read-only so runtime client writes are rejected
      auto* eguProp = getEqFct()->find_property(this->basename() + ".EGU");
      if(eguProp != nullptr) {
        eguProp->set_ro_access();
      }
    }
  }

  /********************************************************************************************************************/

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::auto_init() {
    doocsAdapter.beforeAutoInit();

    DOOCS_T::auto_init();

    // Apply EGU axis configuration from XML after DOOCS loaded persisted values from .conf.
    // If the EGU is defined in XML, the .EGU sub-property is made read-only to prevent
    // runtime modification by clients. On first start the XML value is applied; on
    // subsequent restarts the XML value is compared with the .conf value and re-applied
    // if different.
    applyAxisConfig();
    // send the current value to the device
    // property is writeable OR the target accessor is writable and the only one connected to this property
    // The second case is to have bi-directional variables that are used to persist settings into the config file
    // and need that value back on start-up but are not supposed to be written by the control system and mapped
    // read-only in the configuration file.
    if(this->get_access() == 1 || (_processScalar.isWriteable() && !hasOtherPropertiesToUpdate())) {
      _processScalar = DOOCS_T::value();
      _processScalar.write();
      // set DOOCS time stamp, workaround for DOOCS bug (get() always gives current time stamp if no timestamp is set,
      // which breaks consistency check in ZeroMQ subscriptions after the 4 minutes timeout)
      DOOCS_T::set_stamp();

      updateOthers(false);
    }
  }

  /********************************************************************************************************************/

  template<typename T, typename DOOCS_T>
  void DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    // Note: we already own the location lock by specification of the
    // DoocsUpdater
    T data = _processScalar;

    auto archiverStatus = ArchiveStatus::sts_ok;
    if(_processScalar.dataValidity() != ChimeraTK::DataValidity::ok) {
      archiverStatus = ArchiveStatus::sts_err;
      // set data invalid in DOOCS for current data
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    doocs::EventId eventId;
    if(_macroPulseNumberSource.isInitialised()) {
      eventId = doocs::EventId(_macroPulseNumberSource);
    }
    this->set_value(data, timestamp, eventId, archiverStatus);
    sendZMQ(timestamp);
  }

  /********************************************************************************************************************/

} // namespace ChimeraTK
