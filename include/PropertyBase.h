// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

#include <eq_fct.h>

namespace ChimeraTK {
  class DoocsUpdater;
  class PropertyBase;

  using CommonlyUpdatedPropertySet = std::set<boost::weak_ptr<PropertyBase>>;

  /**
   * Base class used for all properties.
   * Handles data consistency, ZMQ send, and synchronization with other DOOCS buffers
   * All derived classes also must derive from DOOCS D_fct implementation.
   */
  class PropertyBase : public boost::enable_shared_from_this<PropertyBase> {
   public:
    PropertyBase(std::string doocsPropertyName, DoocsUpdater& updater, DataConsistencyGroup::MatchingMode matchingMode);
    virtual ~PropertyBase() = default;

    /// returns associated DOOCS location
    EqFct* getEqFct() { return getDfct()->get_eqfct(); }
    /// returns associated DOOCS property. Not null.
    D_fct* getDfct() { return dynamic_cast<D_fct*>(this); }
    /// turns on ZeroMQ publishing
    void publishZeroMQ() { _publishZMQ = true; }
    /// set macro pulse number source, if configured
    void setMacroPulseNumberSource(const std::string& sourcePath);
    void setMacroPulseNumberSource(
        const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>& macroPulseNumberSource);

    /// Returns list of properties which need to update their DOOCS buffers when any of them changes.
    /// This is used to synchronise multi-mapped PVs.
    CommonlyUpdatedPropertySet& propertiesToUpdate();

   protected:
    CommonlyUpdatedPropertySet _propertiesToUpdate_cache;
    bool _propertiesToUpdate_cacheIsFinal = false;

   public:
    bool hasOtherPropertiesToUpdate() { return propertiesToUpdate().size() > 1; }

   protected:
    /// Update DOOCS buffer from PVs. The given transferElementId shall be used only for checking consistency with the
    /// DataConsistencyGroup. {} will be passed if the update is coming from another property (hence the update will
    /// only be taken with data matching set to none, which is the expected behaviour).
    virtual void updateDoocsBuffer(const TransferElementID& transferElementId) = 0;

    /// should be called for output vars mapped to doocs
    /// registers processVar in data consistency group, initializes DOOCS error state and keeps a reference as
    /// _mainOutputVar. processVar must be owned by this PropertyBase instance for the entire lifetime of the object.
    void setupOutputVar(TransferElementAbstractor& processVar);

    /// register a variable in consistency group
    /// If update=true, updates are processed with our updateDoocsBuffer function.
    void registerVariable(TransferElementAbstractor& var, bool update = true);
    /// update for data consistency group
    bool updateConsistency(const TransferElementID& updatedId);
    /// default implementation returns timestamp of _outputVarForVersionNum
    virtual doocs::Timestamp getTimestamp();
    /// implements timestamp workarounds for associated DOOCS property
    doocs::Timestamp correctDoocsTimestamp();

    /// send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    void sendZMQ(doocs::Timestamp timestamp);

    /// make sure other properties using these PVs see the update
    void updateOthers(bool handleLocking);

    /// a helper which unifies data->device for DOOCS_T = one of D_array<DOOCS_PRIMITIVE_T> or D_spectrum
    template<typename SELF, typename UserType>
    void sendArrayToDevice(SELF* dfct, OneDRegisterAccessor<UserType>& processArray);

    ScalarRegisterAccessor<int64_t> _macroPulseNumberSource;
    DataConsistencyGroup _consistencyGroup;

    std::string _doocsPropertyName;
    DoocsUpdater& _doocsUpdater; // store the reference to the updater. We need it when adding the macro pulse number
    bool _publishZMQ{false};
    // We keep a pointer to the main output var in order to access meta info like VersionNumbers.
    // Storing a plain pointer is ok here (even though the target is essentially a shared_ptr), since the pointer
    // target is owned by the same object (derived class).
    TransferElementAbstractor* _outputVarForVersionNum{nullptr};
    bool _doocsSuccessfullyUpdated{true}; // to detect data losses
    // counter used to reduce amount of data loss warnings printed at console
    size_t _nDataLossWarnings{0};
  };

  /********************************************************************************************************************/

  inline void PropertyBase::setupOutputVar(TransferElementAbstractor& processVar) {
    registerVariable(processVar);
    _outputVarForVersionNum = &processVar;

    if(processVar.isReadable() && !processVar.isWriteable()) {
      // put variable into error state, until a valid value has been received
      getDfct()->d_error(stale_data);
    }
  }

  template<typename SELF, typename UserType>
  void PropertyBase::sendArrayToDevice(SELF* dfct, OneDRegisterAccessor<UserType>& processArray) {
    constexpr bool isSpectrum = std::is_base_of<D_spectrum, SELF>::value;

    // always get a fresh reference
    auto processVector = processArray.data();
    size_t arraySize = processArray.getNElements();
    auto doocsLen = static_cast<size_t>(dfct->length());
    if(doocsLen != arraySize) {
      std::cout << "Warning: Array length mismatch in property " << this->getEqFct()->name() << "/" << dfct->basename()
                << ": Property has length " << doocsLen << " but " << arraySize << " expected." << std::endl;
      arraySize = std::min(arraySize, size_t(doocsLen));
    }
    if constexpr(isSpectrum) {
      // FIXME: find the efficient, memcopying function for float
      // always get a fresh reference
      for(size_t i = 0; i < arraySize; ++i) {
        processVector[i] = dfct->read_spectrum((int)i);
      }
    }
    else {
      // Brute force implementation with a loop. Works for all data types.
      for(size_t i = 0; i < arraySize; ++i) {
        processVector[i] = dfct->value(i);
      }
    }
    auto timestamp = dfct->get_timestamp().to_time_point();
    processArray.write(VersionNumber(timestamp));

    // Correct property length in case of a mismatch.
    if(doocsLen != arraySize) {
      if constexpr(isSpectrum) {
        dfct->length(arraySize);
        // FIXME - do we need to restore values like for arrays?
        // it's more difficult with D_spectrum because of the buffered/unbuffered feature
      }
      else {
        dfct->set_length(arraySize);
        // restore value from ProcessArray, as it may have been destroyed in set_length().
        for(size_t i = 0; i < arraySize; ++i) {
          dfct->set_value(processVector[i], i);
        }
      }
    }
  }

} // namespace ChimeraTK
