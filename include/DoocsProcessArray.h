// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

#include <D_spectrum.h>
#include <eq_fct.h>

namespace ChimeraTK {

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  class DoocsProcessArray : public DOOCS_T, public boost::noncopyable, public PropertyBase {
   public:
    // We have to cast the pointer to the correct underlying DOOCS type. No real conversion is done, since only
    // equivalent types are casted here.
    using THE_DOOCS_TYPE = typename std::result_of<decltype (&DOOCS_T::value)(DOOCS_T, int)>::type;

    DoocsProcessArray(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>> const& processArray, DoocsUpdater& updater,
        DataConsistencyGroup::MatchingMode matchingMode);

    /**
     * Overload the set function which is called by DOOCS to inject sending to the
     * device.
     */
    void set(EqAdr* eqAdr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqFct) override;

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init() override;

    /// Flag whether the value has been modified since the content has been saved to disk the last time
    /// (see CSAdapterEqFct::saveArray()).
    bool modified{false};

   protected:
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    OneDRegisterAccessor<DOOCS_PRIMITIVE_T> _processArray;

    // Internal function which copies the content from the DOOCS container into
    // the ChimeraTK ProcessArray and calls the send method. Factored out to allow
    // unit testing.
    void sendToDevice(bool getLocks);
  };

  /********************************************************************************************************************/

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::DoocsProcessArray(EqFct* eqFct, const std::string& doocsPropertyName,
      const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>>& processArray, DoocsUpdater& updater,
      DataConsistencyGroup::MatchingMode matchingMode)
  : DOOCS_T(doocsPropertyName, processArray->getNumberOfSamples(), eqFct),
    PropertyBase(doocsPropertyName, updater, matchingMode), _processArray(processArray) {
    // Check if the array length exceeds the maximum allowed length by DOOCS.
    // DOOCS does not report this as an error and instead silently truncates the
    // array length.
    auto len = static_cast<size_t>(this->length());
    if(processArray->getNumberOfSamples() != len) {
      std::stringstream s;
      s << "Error: The selected DOOCS data type for the variable '" << processArray->getName() << "' "
        << "(mapped to the DOOCS name '" << _doocsPropertyName << "') seems not to support the requested length of "
        << processArray->getNumberOfSamples() << " since the DOOCS property has a length of " << len
        << ". Try selecting a different DOOCS type in the mappng XML file, e.g. a D_spectrum!";
      throw ChimeraTK::logic_error(s.str());
    }
    setupOutputVar(_processArray);
  }

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  void DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::set(
      EqAdr* eqAdr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqFct) {
    DOOCS_T::set(eqAdr, data1, data2, eqFct);
    if(_macroPulseNumberSource.isInitialised()) {
      this->set_mpnum(_macroPulseNumberSource);
    }
    modified = true;
    sendToDevice(true);
    sendZMQ(getTimestamp());
  }

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  void DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::auto_init() {
    doocsAdapter.beforeAutoInit();

    DOOCS_T::auto_init();
    modified = false;
    // send the current value to the device
    // property is writeable OR the target accessor is writable and the only one connected to this property
    // The second case is to have bi-directional variables that are used to persist settings into the config file
    // and need that value back on start-up but are not supposed to be written by the control system and mapped
    // read-only in the configuration file.
    if(this->get_access() == 1 ||
        (_processArray.isWriteable() && !hasOtherPropertiesToUpdate())) { // property is writeable
      sendToDevice(false);
      // set DOOCS time stamp, workaround for DOOCS bug (get() always gives current time stamp if no timestamp is set,
      // which breaks consistency check in ZeroMQ subscriptions after the 4 minutes timeout)
      DOOCS_T::set_stamp();
    }
  }

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  void DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    // Note: we already own the location lock by specification of the
    // DoocsUpdater
    THE_DOOCS_TYPE* dataPtr;
    if constexpr(std::is_same<THE_DOOCS_TYPE, DOOCS_PRIMITIVE_T>::value) {
      // No cast necessary if types are identical.
      dataPtr = _processArray.data();
    }
    else if constexpr(std::is_same<DOOCS_PRIMITIVE_T, ChimeraTK::Boolean>::value &&
        std::is_same<THE_DOOCS_TYPE, bool>::value) {
      // FIXME: Is it really ok to use reinterpret_cast here?
      static_assert(sizeof(ChimeraTK::Boolean) == sizeof(bool));
      dataPtr = reinterpret_cast<bool*>(_processArray.data());
    }
    else {
      static_assert(std::is_same<THE_DOOCS_TYPE, DOOCS_PRIMITIVE_T>::value, "Bad type casting.");
    }

    if(_processArray.dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    this->fill_array(dataPtr, _processArray.getNElements());
    modified = true;

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    if(_macroPulseNumberSource.isInitialised()) {
      this->set_mpnum(_macroPulseNumberSource);
    }

    sendZMQ(timestamp);
  }

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  void DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::sendToDevice(bool getLocks) {
    sendArrayToDevice(this, _processArray);
    updateOthers(getLocks);
  }

} // namespace ChimeraTK
