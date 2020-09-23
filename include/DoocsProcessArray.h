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
          << ". Try selectin a different DOOCS type in the mappng XML file, e.g. "
             "a D_spectrum!";
        throw ChimeraTK::logic_error(s.str());
      }
    }

    /**
     * Overload the set function which is called by DOOCS to inject sending to the
     * device.
     */
    void set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) override {
      DOOCS_T::set(eqAdr, data1, data2, eqFct);
      sendToDevice();
      // send data via ZeroMQ if enabled
      if(publishZMQ) {
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
        this->send(&info);
      }
    }

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init(void) override {
      DOOCS_T::auto_init();
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

      // We have to cast the pointer to the correct underlying DOOCS type. This
      // cast never does anything, the only reason is to "convert" from int64_t to
      // long long int (which are different types!)
      typedef typename std::result_of<decltype (&DOOCS_T::value)(DOOCS_T, int)>::type THE_DOOCS_TYPE;
      static_assert(std::is_same<THE_DOOCS_TYPE, DOOCS_PRIMITIVE_T>::value ||
              (std::is_same<DOOCS_PRIMITIVE_T, int64_t>::value && std::is_same<THE_DOOCS_TYPE, long long int>::value),
          "Bad type casting.");
      auto dataPtr = reinterpret_cast<THE_DOOCS_TYPE*>(processVector.data());

      if(_processArray->dataValidity() != ChimeraTK::DataValidity::ok) {
        this->d_error(stale_data);
      }
      else {
        this->d_error(no_error);
      }

      this->fill_array(dataPtr, processVector.size());

      // Convert time stamp from version number in Unix time (seconds and microseconds).
      // Note that epoch of std::chrono::system_time might be different from Unix time, and Unix time omits leap seconds
      // and hence is not the duration since the epoch! We have to convert to time_t and then find out the microseconds.
      auto timestamp = _processArray->getVersionNumber().getTime();
      auto seconds = std::chrono::system_clock::to_time_t(timestamp);
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
          timestamp - std::chrono::system_clock::from_time_t(seconds))
                              .count();
      this->set_tmstmp(seconds, microseconds);
      if(_macroPulseNumberSource) this->set_mpnum(_macroPulseNumberSource->accessData(0));

      // send data via ZeroMQ if enabled
      if(publishZMQ) {
        dmsg_info info;
        memset(&info, 0, sizeof(info));
        info.sec = seconds;
        info.usec = microseconds;
        info.ident = _macroPulseNumberSource->accessData(0);
        auto ret = this->send(&info);
        if(ret) {
          std::cout << "ZeroMQ sending failed!!!" << std::endl;
        }
      }
    }

    void publishZeroMQ() { publishZMQ = true; }

    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
      if(_processArray->isReadable()) {
        _macroPulseNumberSource = macroPulseNumberSource;
        if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
          _consistencyGroup.add(macroPulseNumberSource);
          _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct,
              std::bind(&DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::updateDoocsBuffer, this,
                  macroPulseNumberSource->getId()));
        }
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
