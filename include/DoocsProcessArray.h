#ifndef __DOOCS_PROCESS_ARRAY_H__
#define __DOOCS_PROCESS_ARRAY_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h> // needed for the macro pulse number
#include <ChimeraTK/DataConsistencyGroup.h>

#include "DoocsUpdater.h"
#include "DoocsAdapter.h"
#include "splitStringAtFirstSlash.h"

#include <eq_fct.h>

namespace ChimeraTK {

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  class DoocsProcessArray : public DOOCS_T, public boost::noncopyable {
   public:
    DoocsProcessArray(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>> const& processArray, DoocsUpdater& updater)
    : DOOCS_T(doocsPropertyName.c_str(), processArray->getNumberOfSamples(), eqFct), _processArray(processArray),
      _doocsUpdater(updater), _eqFct(eqFct) {
      if(processArray->isReadable()) {
        updater.addVariable(ChimeraTK::OneDRegisterAccessor<DOOCS_PRIMITIVE_T>(processArray), eqFct,
            std::bind(&DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::updateDoocsBuffer, this, processArray->getId()));
        _consistencyGroup.add(processArray);
      }

      // Check if the array length exceeds the maximum allowed length by DOOCS.
      // DOOCS does not report this as an error and instead silently truncates the
      // array length.
      if(processArray->getNumberOfSamples() != static_cast<size_t>(this->length())) {
        std::stringstream s;
        s << "Error: The selected DOOCS data type for the variable '" << processArray->getName() << "' "
          << "(mapped to the DOOCS name '" << doocsPropertyName << "') seems not to support the requested length of "
          << processArray->getNumberOfSamples() << " since the DOOCS property has a length of " << this->length()
          << ". Try selecting a different DOOCS type in the mappng XML file, e.g. a D_spectrum!";
        throw ChimeraTK::logic_error(s.str());
      }

      // put variable into error state, until a valid value has been received
      if(!_processArray->isWriteable()) {
        this->d_error(stale_data);
      }
    }

    /**
     * Overload the set function which is called by DOOCS to inject sending to the
     * device.
     */
    void set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) override {
      DOOCS_T::set(eqAdr, data1, data2, eqFct);
      modified = true;
      sendToDevice();
      // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
      if(publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
        auto timestamp = _processArray->getVersionNumber().getTime();
        auto seconds = std::chrono::system_clock::to_time_t(timestamp);
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
            timestamp - std::chrono::system_clock::from_time_t(seconds))
                                .count();
        dmsg_info info;
        memset(&info, 0, sizeof(info));
        info.sec = seconds;
        info.usec = microseconds;
        if(_macroPulseNumberSource != nullptr) {
          info.ident = _macroPulseNumberSource->accessData(0);
        }
        else {
          info.ident = 0;
        }
        auto ret = this->send(&info);
        if(ret) {
          std::cout << "ZeroMQ sending failed!!!" << std::endl;
        }
      }
    }

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init(void) override {
      DOOCS_T::auto_init();
      modified = false;
      // send the current value to the device
      if(_processArray->isWriteable()) {
        sendToDevice();
        // set DOOCS time stamp, workaround for DOOCS bug (get() always gives current time stamp if no timestamp is set,
        // which breaks consistency check in ZeroMQ subscriptions after the 4 minutes timeout)
        DOOCS_T::set_stamp();
      }
    }

    void updateDoocsBuffer(TransferElementID transferElementId) {
      // FIXME: A first  implementation is checking the data consistency here. Later this should be
      // before calling this function because calling this function through a function pointer is
      // comparatively expensive.
      // Only check the consistency group if there is a macro pulse number associated.
      if(_macroPulseNumberSource && !_consistencyGroup.update(transferElementId)) {
        // data is not consistent (yet). Don't update the Doocs buffer.
        // check if this will now throw away data and generate a warning
        if(transferElementId == _processArray->getId()) {
          if(!_doocsSuccessfullyUpdated) {
            ++_nDataLossWarnings;
            if(DoocsAdapter::checkPrintDataLossWarning(_nDataLossWarnings)) {
              std::cout << "WARNING: Data loss in array property " << _eqFct->name() << "/" << this->basename()
                        << " due to failed data matching between value and macro pulse number (repeated "
                        << _nDataLossWarnings << " times)." << std::endl;
            }
          }
        }
        _doocsSuccessfullyUpdated = false;
        return;
      }
      _doocsSuccessfullyUpdated = true;

      // Note: we already own the location lock by specification of the
      // DoocsUpdater
      auto& processVector = _processArray->accessChannel(0);

      // We have to cast the pointer to the correct underlying DOOCS type. No real conversion is done, since only
      // equivalent types are casted here.
      typedef typename std::result_of<decltype (&DOOCS_T::value)(DOOCS_T, int)>::type THE_DOOCS_TYPE;
      THE_DOOCS_TYPE* dataPtr;
      if constexpr(std::is_same<THE_DOOCS_TYPE, DOOCS_PRIMITIVE_T>::value) {
        // No cast necessary if types are identical.
        dataPtr = processVector.data();
      }
      else if constexpr(std::is_same<DOOCS_PRIMITIVE_T, int64_t>::value &&
          std::is_same<THE_DOOCS_TYPE, long long int>::value) {
        // Cast from int64_t to long long int (which are different types!)
        dataPtr = reinterpret_cast<long long int*>(processVector.data());
      }
      else if constexpr(std::is_same<DOOCS_PRIMITIVE_T, ChimeraTK::Boolean>::value &&
          std::is_same<THE_DOOCS_TYPE, bool>::value) {
        // FIXME: Is it really ok to use reinterpret_cast here?
        static_assert(sizeof(ChimeraTK::Boolean) == sizeof(bool));
        dataPtr = reinterpret_cast<bool*>(processVector.data());
      }
      else {
        static_assert(std::is_same<THE_DOOCS_TYPE, DOOCS_PRIMITIVE_T>::value, "Bad type casting.");
      }

      if(_processArray->dataValidity() != ChimeraTK::DataValidity::ok) {
        this->d_error(stale_data);
      }
      else {
        this->d_error(no_error);
      }

      this->fill_array(dataPtr, processVector.size());
      modified = true;

      // Convert time stamp from version number to DOOCS timestamp
      doocs::Timestamp timestamp(_processArray->getVersionNumber().getTime());

      // Make sure we never send out two absolute identical time stamps. If we would do so, the "watchdog" which
      // corrects inconsistencies in ZeroMQ subscriptions between sender and subcriber cannot detect the inconsistency.
      if(this->get_timestamp() == timestamp) {
        timestamp += std::chrono::microseconds(1);
      }

      this->set_timestamp(timestamp);
      if(_macroPulseNumberSource) this->set_mpnum(_macroPulseNumberSource->accessData(0));

      // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
      if(publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
        dmsg_info info;
        memset(&info, 0, sizeof(info));
        auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
        info.sec = sinceEpoch.seconds;
        info.usec = sinceEpoch.microseconds;
        if(_macroPulseNumberSource != nullptr) {
          info.ident = _macroPulseNumberSource->accessData(0);
        }
        else {
          info.ident = 0;
        }
        auto ret = this->send(&info);
        if(ret) {
          std::cout << "ZeroMQ sending failed!!!" << std::endl;
        }
      }
    }

    void publishZeroMQ() { publishZMQ = true; }

    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
        _macroPulseNumberSource = macroPulseNumberSource;
        if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
          _consistencyGroup.add(macroPulseNumberSource);
          _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct,
              std::bind(&DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::updateDoocsBuffer, this,
                  macroPulseNumberSource->getId()));
        }
        else {
          // We don't need to match up anything with it when it changes, but we have to register this at least once
          // so the macropulse number will be included in the readAnyGroup in the updater if
          // <data_matching> is none everywhere
          _doocsUpdater.addVariable(
              ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct, []() {});
        }
    }

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>> _processArray;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
    DataConsistencyGroup _consistencyGroup;
    DoocsUpdater& _doocsUpdater; // store the reference to the updater. We need it when adding the macro pulse number
    EqFct* _eqFct;               // We need it when adding the macro pulse number
    bool publishZMQ{false};
    bool _doocsSuccessfullyUpdated{true}; // to detect data losses

    // counter used to reduce amount of data loss warnings printed at console
    size_t _nDataLossWarnings{0};

    // Flag whether the value has been modified since the content has been saved to disk the last time
    // (see CSAdapterEqFct::saveArray()).
    bool modified{false};

    // Internal function which copies the content from the DOOCS container into
    // the ChimeraTK ProcessArray and calls the send method. Factored out to allow
    // unit testing.
    void sendToDevice() {
      // Brute force implementation with a loop. Works for all data types.
      // always get a fresh reference
      auto& processVector = _processArray->accessChannel(0);
      size_t arraySize = processVector.size();
      for(size_t i = 0; i < arraySize; ++i) {
        processVector[i] = this->value(i);
      }
      _processArray->write();
    }
  };

} // namespace ChimeraTK

#endif // __DOOCS_PROCESS_ARRAY_H__
