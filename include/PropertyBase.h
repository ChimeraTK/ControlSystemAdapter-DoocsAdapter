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

  /**
   * Base class used for all properties.
   * Handles data consistency, ZMQ send, and synchronization with other DOOCS buffers
   * All derived classes also must derive from DOOCS D_fct implementation.
   */
  class PropertyBase : public boost::enable_shared_from_this<PropertyBase> {
   public:
    PropertyBase(std::string doocsPropertyName, DoocsUpdater& updater)
    : _doocsPropertyName(std::move(doocsPropertyName)), _doocsUpdater(updater) {}
    virtual ~PropertyBase() = default;

    /// returns associated DOOCS location
    EqFct* getEqFct() { return getDfct()->get_eqfct(); }
    /// returns associated DOOCS property. Not null.
    D_fct* getDfct() { return dynamic_cast<D_fct*>(this); }
    /// turns on ZeroMQ publishing
    void publishZeroMQ() { _publishZMQ = true; }
    void setMacroPulseNumberSource(
        const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>& macroPulseNumberSource);
    void setMatchingMode(DataConsistencyGroup::MatchingMode newMode) { _consistencyGroup.setMatchingMode(newMode); }

    /// List of other properties which need to update their DOOCS buffers when this property is written from the DOOCS
    /// side. This is used to synchronise multi-mapped PVs.
    std::set<boost::weak_ptr<PropertyBase>> otherPropertiesToUpdate;

   protected:
    /// Update DOOCS buffer from PVs. The given transferElementId shall be used only for checking consistency with the
    /// DataConsistencyGroup. {} will be passed if the update is coming from another property (hence the update will
    /// only be taken with data matching set to none, which is the expected behaviour).
    virtual void updateDoocsBuffer(const TransferElementID& transferElementId) = 0;

    /// should be called for output vars mapped to doocs
    /// registers processVar in data consistency group, initializes DOOCS error state and keeps a reference as _mainOutputVar
    template<typename T>
    void setupOutputVar(boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processVar);

    /// register a variable in consistency group
    void registerVariable(const ChimeraTK::TransferElementAbstractor& var);
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
    void sendArrayToDevice(SELF* dfct, const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<UserType>>& processArray);

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
    DataConsistencyGroup _consistencyGroup;

    std::string _doocsPropertyName;
    DoocsUpdater& _doocsUpdater; // store the reference to the updater. We need it when adding the macro pulse number
    bool _publishZMQ{false};
    // we keep a pointer to the main output var in order to access meta info like VersionNumbers
    boost::shared_ptr<ChimeraTK::TransferElement> _outputVarForVersionNum;
    bool _doocsSuccessfullyUpdated{true}; // to detect data losses
    // counter used to reduce amount of data loss warnings printed at console
    size_t _nDataLossWarnings{0};
  };

  /*****************************************************************************/

  template<typename T>
  void PropertyBase::setupOutputVar(boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processVar) {
    registerVariable(TransferElementAbstractor{processVar});
    _outputVarForVersionNum = processVar;

    if(processVar->isReadable() && !processVar->isWriteable()) {
      // put variable into error state, until a valid value has been received
      getDfct()->d_error(stale_data);
    }
  }

  template<typename SELF, typename UserType>
  void PropertyBase::sendArrayToDevice(
      SELF* dfct, const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<UserType>>& processArray) {
    constexpr bool isSpectrum = std::is_base_of<D_spectrum, SELF>::value;

    // always get a fresh reference
    auto& processVector = processArray->accessChannel(0);
    size_t arraySize = processVector.size();
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
    processArray->write(VersionNumber(timestamp));

    // Correct property length in case of a mismatch.
    if(doocsLen != processVector.size()) {
      if constexpr(isSpectrum) {
        dfct->length(processVector.size());
        // FIXME - do we need to restore values like for arrays?
        // it's more difficult with D_spectrum because of the buffered/unbuffered feature
      }
      else {
        dfct->set_length(processVector.size());
        // restore value from ProcessArray, as it may have been destroyed in set_length().
        for(size_t i = 0; i < arraySize; ++i) {
          dfct->set_value(processVector[i], i);
        }
      }
    }
  }

} // namespace ChimeraTK
