#ifndef __DOOCS_PROCESS_ARRAY_H__
#define __DOOCS_PROCESS_ARRAY_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include <ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariableListener.h>
#include <ChimeraTK/OneDRegisterAccessor.h>

#include "splitStringAtFirstSlash.h"

#include <eq_fct.h>

namespace ChimeraTK {

template <typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
class DoocsProcessArray : public DOOCS_T, public boost::noncopyable {
public:
  DoocsProcessArray(
      EqFct *eqFct, std::string const &doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>> const
          &processArray,
      DoocsUpdater &updater)
      : DOOCS_T(doocsPropertyName.c_str(), processArray->getNumberOfSamples(),
                eqFct),
        _processArray(processArray) {
    if (processArray->isReadable()) {
      updater.addVariable(
          ChimeraTK::OneDRegisterAccessor<DOOCS_PRIMITIVE_T>(processArray),
          eqFct,
          std::bind(
              &DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::updateDoocsBuffer,
              this));
    }

    // Check if the array length exceeds the maximum allowed length by DOOCS.
    // DOOCS does not report this as an error and instead silently truncates the
    // array length.
    if (processArray->getNumberOfSamples() !=
        static_cast<size_t>(this->length())) {
      std::stringstream s;
      s << "Error: The selected DOOCS data type for the variable '"
        << processArray->getName() << "' "
        << "(mapped to the DOOCS name '" << doocsPropertyName
        << "') seems not to support the requested length of "
        << processArray->getNumberOfSamples()
        << " since the DOOCS property has a length of " << this->length()
        << ". Try selectin a different DOOCS type in the mappng XML file, e.g. "
           "a D_spectrum!";
      throw ChimeraTK::logic_error(s.str());
    }
  }

  /**
   * Overload the set function which is called by DOOCS to inject sending to the
   * device.
   */
  void set(EqAdr *eqAdr, EqData *data1, EqData *data2, EqFct *eqFct) override {
    DOOCS_T::set(eqAdr, data1, data2, eqFct);
    sendToDevice();
  }

  /**
   * Override the Doocs auto_init() method, which is called after initialising
   * the value of the property from the config file.
   */
  void auto_init(void) override {
    DOOCS_T::auto_init();
    // send the current value to the device
    if (_processArray->isWriteable()) {
      sendToDevice();
    }
  }

  void updateDoocsBuffer() {
    // Note: we already own the location lock by specification of the
    // DoocsUpdater
    auto &processVector = _processArray->accessChannel(0);

    // We have to cast the pointer to the correct underlying DOOCS type. This
    // cast never does anything, the only reason is to "convert" from int64_t to
    // long long int (which are different types!)
    typedef
        typename std::result_of<decltype (&DOOCS_T::value)(DOOCS_T, int)>::type
            THE_DOOCS_TYPE;
    static_assert(std::is_same<THE_DOOCS_TYPE, DOOCS_PRIMITIVE_T>::value ||
                      (std::is_same<DOOCS_PRIMITIVE_T, int64_t>::value &&
                       std::is_same<THE_DOOCS_TYPE, long long int>::value),
                  "Bad type casting.");
    auto dataPtr = reinterpret_cast<THE_DOOCS_TYPE *>(processVector.data());

    this->fill_array(dataPtr, processVector.size());
    if (publishZMQ) {
      dmsg_info info;
      memset(&info, 0, sizeof(info));
      auto sinceEpoch =
          _processArray->getVersionNumber().getTime().time_since_epoch();
      auto time =
          std::chrono::duration_cast<std::chrono::microseconds>(sinceEpoch);
      info.sec = time.count() / 1000000;
      info.usec = time.count() % 1000000;
      info.ident = _macroPulseNumberSource->accessData(0);
      this->send(&info);
    }
  }

  void publishZeroMQ() { publishZMQ = true; }

  void setMacroPulseNumberSource(
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>
          macroPulseNumberSource) {
    _macroPulseNumberSource = macroPulseNumberSource;
  }

protected:
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>>
      _processArray;
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>
      _macroPulseNumberSource;
  bool publishZMQ{false};

  // Internal function which copies the content from the DOOCS container into
  // the ChimeraTK ProcessArray and calls the send method. Factored out to allow
  // unit testing.
  void sendToDevice() {
    // Brute force implementation with a loop. Works for all data types.
    // always get a fresh reference
    auto &processVector = _processArray->accessChannel(0);
    size_t arraySize = processVector.size();
    for (size_t i = 0; i < arraySize; ++i) {
      processVector[i] = this->value(i);
    }
    _processArray->write();
  }
};

} // namespace ChimeraTK

#endif // __DOOCS_PROCESS_ARRAY_H__
