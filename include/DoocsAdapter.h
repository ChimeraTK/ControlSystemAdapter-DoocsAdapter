// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"

#include <ChimeraTK/DataConsistencyGroup.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

namespace ChimeraTK {

  /**
   * Base class used for all properties.
   * Handles data consistency, ZMQ send, and synchronization with other DOOCS buffers
   */
  class PropertyBase : public boost::enable_shared_from_this<PropertyBase> {
   public:
    PropertyBase(EqFct* eqFct, std::string doocsPropertyName, DoocsUpdater& updater)
    : _eqFct(eqFct), _doocsPropertyName(doocsPropertyName), _doocsUpdater(updater) {}

    virtual EqFct* getEqFct() { return _eqFct; }
    D_fct* getDfct() { return dynamic_cast<D_fct*>(this); }
    virtual void publishZeroMQ() { _publishZMQ = true; }
    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource);
    void setMatchingMode(DataConsistencyGroup::MatchingMode newMode) { _consistencyGroup.setMatchingMode(newMode); }

   protected:
    /// Update DOOCS buffer from PVs. The given transferElementId shall be used only for checking consistency with the
    /// DataConsistencyGroup. {} will be passed if the update is coming from another property (hence the update will
    /// only be taken with data matching set to none, which is the expected behaviour).
    virtual void updateDoocsBuffer(const TransferElementID& transferElementId) = 0;

    /// should be called for output vars mapped to doocs
    template<typename T>
    void init(boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processVar);

    /// register a variable in consistency group
    void registerVariable(const ChimeraTK::TransferElementAbstractor& var);

    bool updateConsistency(const TransferElementID& updatedId);
    virtual doocs::Timestamp getTimestamp();
    doocs::Timestamp correctDoocsTimestamp();

    /// send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    void sendZMQ(doocs::Timestamp timestamp);

    /// make sure other properties using these PVs see the update
    void updateOthers(bool handleLocking);

   public:
    /// List of other properties which need to update their DOOCS buffers when this property is written from the DOOCS
    /// side. This is used to synchronise multi-mapped PVs.
    std::set<boost::shared_ptr<PropertyBase>> otherPropertiesToUpdate;

   protected:
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
    DataConsistencyGroup _consistencyGroup;

    EqFct* _eqFct; // We need it when adding the macro pulse number
    std::string _doocsPropertyName;
    DoocsUpdater& _doocsUpdater; // store the reference to the updater. We need it when adding the macro pulse number
    bool _publishZMQ{false};
    // we keep a pointer to the main output var in order to access meta info like VersionNumbers
    boost::shared_ptr<ChimeraTK::TransferElement> _mainOutputVar;
    bool _doocsSuccessfullyUpdated{true}; // to detect data losses
    // counter used to reduce amount of data loss warnings printed at console
    size_t _nDataLossWarnings{0};
  };

  /********************************************************************************/

  /** The main adapter class. With this tool the EqFct should shrink to about 4
   * lines of code (plus boiler plate).
   */
  class DoocsAdapter {
   public:
    DoocsAdapter();
    boost::shared_ptr<DevicePVManager> const& getDevicePVManager() const;
    boost::shared_ptr<ControlSystemPVManager> const& getControlSystemPVManager() const;

    boost::shared_ptr<DoocsUpdater> updater;

    // Function to be called in all auto_init() implementations, to initialise otherPropertiesToUpdate lists in all
    // properties. This needs to be done after all locations have been created but before the properties get their
    // initial values from the config file. DOOCS seems not to provide any hook at that point... This function will only
    // perform an action when called for the first time.
    void before_auto_init();

    // An atomic bool which is set true in post_init_epilog to indicate that doocs
    // is ready. Only used in testing.
    static std::atomic<bool> isInitialised;

    // A convenience function to wait until the adapter is initialised.
    static void waitUntilInitialised();

    // Function used by the property implementations to decide whether to print a "data loss" warning
    static bool checkPrintDataLossWarning(size_t counter);

    // stores list of writable PVs which are mapped to multiple properties
    // Note: this is cleared in post_init_epilog() to save memory.
    std::map<std::string, std::set<boost::shared_ptr<PropertyBase>>> writeableVariablesWithMultipleProperties;

   protected:
    boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager;
    boost::shared_ptr<DevicePVManager> _devicePVManager;

    // flag whether before_auto_init() has already been called.
    bool before_auto_init_called{false};
  };

  /*****************************************************************************/

  template<typename T>
  void PropertyBase::init(boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processVar) {
    registerVariable(TransferElementAbstractor{processVar});
    _mainOutputVar = processVar;

    if(processVar->isReadable() && !processVar->isWriteable()) {
      // put variable into error state, until a valid value has been received
      getDfct()->d_error(stale_data);
    }
  }
} // namespace ChimeraTK

extern ChimeraTK::DoocsAdapter doocsAdapter;
