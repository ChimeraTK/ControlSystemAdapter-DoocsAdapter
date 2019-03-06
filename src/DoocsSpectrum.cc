#include "DoocsSpectrum.h"

#include <eq_fct.h>

#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

namespace ChimeraTK {

  DoocsSpectrum::DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& startAccessor,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& incrementAccessor)
  : D_spectrum(doocsPropertyName.c_str(), processArray->getNumberOfSamples(), eqFct, true), _processArray(processArray),
    _startAccessor(startAccessor), _incrementAccessor(incrementAccessor), nBuffers(1) {
    if(processArray->isReadable()) {
      updater.addVariable(ChimeraTK::OneDRegisterAccessor<float>(processArray), eqFct,
          std::bind(&DoocsSpectrum::updateDoocsBuffer, this));
    }
    if(startAccessor && startAccessor->isReadable()) {
      updater.addVariable(ChimeraTK::ScalarRegisterAccessor<float>(startAccessor), eqFct,
          std::bind(&DoocsSpectrum::updateParameters, this));
    }
    if(incrementAccessor && incrementAccessor->isReadable()) {
      updater.addVariable(ChimeraTK::ScalarRegisterAccessor<float>(incrementAccessor), eqFct,
          std::bind(&DoocsSpectrum::updateParameters, this));
    }
  }

  DoocsSpectrum::DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& startAccessor,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& incrementAccessor, size_t numberOfBuffers)
  : D_spectrum(doocsPropertyName.c_str(), processArray->getNumberOfSamples(), eqFct, numberOfBuffers, DATA_A_FLOAT),
    _processArray(processArray), _startAccessor(startAccessor), _incrementAccessor(incrementAccessor),
    nBuffers(numberOfBuffers) {
    if(processArray->isReadable()) {
      updater.addVariable(ChimeraTK::OneDRegisterAccessor<float>(processArray), eqFct,
          std::bind(&DoocsSpectrum::updateDoocsBuffer, this));
    }
    if(startAccessor && startAccessor->isReadable()) {
      updater.addVariable(ChimeraTK::ScalarRegisterAccessor<float>(startAccessor), eqFct,
          std::bind(&DoocsSpectrum::updateParameters, this));
    }
    if(incrementAccessor && incrementAccessor->isReadable()) {
      updater.addVariable(ChimeraTK::ScalarRegisterAccessor<float>(incrementAccessor), eqFct,
          std::bind(&DoocsSpectrum::updateParameters, this));
    }
  }

  void DoocsSpectrum::set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) {
    D_spectrum::set(eqAdr, data1, data2, eqFct);
    sendToDevice();
  }

  void DoocsSpectrum::get(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) {
    D_spectrum::get(eqAdr, data1, data2, eqFct);
  }

  void DoocsSpectrum::auto_init(void) {
    D_spectrum::read();
    // send the current value to the device
    if(_processArray->isWriteable()) {
      sendToDevice();
    }
  }

  void DoocsSpectrum::updateDoocsBuffer() {
    // Note: we already own the location lock by specification of the DoocsUpdater

    // determine time stamp
    auto sinceEpoch = _processArray->getVersionNumber().getTime().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(sinceEpoch);
    auto sec = time.count() / 1000000;
    auto usec = time.count() % 1000000;

    // set macro pulse number, buffer number and time stamp
    auto ibuf = _macroPulseNumberSource->accessData(0) % nBuffers;
    macro_pulse(_macroPulseNumberSource->accessData(0), ibuf);
    set_tmstmp(sec, usec, ibuf);

    // fill the spectrum
    std::vector<float>& processVector = _processArray->accessChannel(0);
    fill_spectrum(processVector.data(), processVector.size(), ibuf);
    if(publishZMQ) {
      dmsg_info info;
      memset(&info, 0, sizeof(info));
      info.sec = sec;
      info.usec = usec;
      info.ident = _macroPulseNumberSource->accessData(0);
      this->send(&info);
    }
  }

  void DoocsSpectrum::updateParameters() {
    // Note: we already own the location lock by specification of the DoocsUpdater
    float start, increment;
    if(_startAccessor) {
      start = _startAccessor->accessData(0);
    }
    else {
      start = this->spec_start();
    }
    if(_incrementAccessor) {
      increment = _incrementAccessor->accessData(0);
    }
    else {
      increment = this->spec_inc();
    }

    spectrum_parameter(this->spec_time(), start, increment, this->spec_status());
  }

  void DoocsSpectrum::sendToDevice() {
    // Brute force implementation with a loop. Works for all data types.
    // FIXME: find the efficient, memcopying function for float
    // always get a fresh reference
    std::vector<float>& processVector = _processArray->accessChannel(0);
    size_t arraySize = processVector.size();
    for(size_t i = 0; i < arraySize; ++i) {
      processVector[i] = read_spectrum(i);
    }
    _processArray->write();
  }

} // namespace ChimeraTK
