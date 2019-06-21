#include "DoocsSpectrum.h"

#include <eq_fct.h>

#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

namespace ChimeraTK {

  DoocsSpectrum::DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& startAccessor,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& incrementAccessor)
  : D_spectrum(doocsPropertyName.c_str(), processArray->getNumberOfSamples(), eqFct, processArray->isWriteable()),
    _processArray(processArray), _startAccessor(startAccessor), _incrementAccessor(incrementAccessor),
    _doocsUpdater(updater), _eqFct(eqFct), nBuffers(1) {
    if(processArray->isReadable()) {
      updater.addVariable(ChimeraTK::OneDRegisterAccessor<float>(processArray), eqFct,
          std::bind(&DoocsSpectrum::updateDoocsBuffer, this, processArray->getId()));
      _consistencyGroup.add(processArray);
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
    _doocsUpdater(updater), _eqFct(eqFct), nBuffers(numberOfBuffers) {
    if(nBuffers > 1 && !processArray->isReadable()) {
      throw ChimeraTK::logic_error(
          "D_spectrum '" + _processArray->getName() + "' has numberOfBuffers > 1 but is not readable.");
    }
    if(processArray->isReadable()) {
      updater.addVariable(ChimeraTK::OneDRegisterAccessor<float>(processArray), eqFct,
          std::bind(&DoocsSpectrum::updateDoocsBuffer, this, processArray->getId()));
      _consistencyGroup.add(processArray);
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
    modified = true;
    sendToDevice();
  }

  void DoocsSpectrum::auto_init(void) {
    // check if the macro pulse number source has been set if the spectrum is buffered
    if(nBuffers > 1) {
      if(_macroPulseNumberSource == nullptr) {
        throw ChimeraTK::logic_error(
            "D_spectrum '" + _processArray->getName() + "' has numberOfBuffers > 1 but not macro pulse number source.");
      }
    }

    // send the current value to the device
    D_spectrum::read();
    modified = false;
    if(_processArray->isWriteable()) {
      sendToDevice();
    }
  }

  void DoocsSpectrum::write(fstream& fptr) {
    // DOOCS is normally keeping the location lock until everything is written for that location: all D_spectrum and all
    // other properties. This can take too long (like seconds), which leads to noticable freezes of the UI. As a
    // work-around we release the lock here, wait some time and acquire the lock again. Since this happens in a separate
    // thread (svr_writer), this slow-down should not be of any harm.
    efp_->unlock();
    usleep(1000);
    efp_->lock();
    if(!modified) return;
    D_spectrum::write(fptr);
  }

  void DoocsSpectrum::updateDoocsBuffer(TransferElementID transferElementId) {
    // Note: we already own the location lock by specification of the DoocsUpdater

    // FIXME: A first  implementation is checking the data consistency here. Later this should be
    // before calling this function because calling this function through a function pointer is
    // comparatively expensive.
    // Only check the consistency group if there is a macro pulse number associated.
    // There are only the processArray and the macro pulse number in the consistency
    // group. The limits are coming asynchronously and not for every macro pulse,
    // so we just take test latest we have.
    if(_macroPulseNumberSource && !_consistencyGroup.update(transferElementId)) {
      return;
    }

    // determine time stamp
    auto sinceEpoch = _processArray->getVersionNumber().getTime().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(sinceEpoch);
    auto sec = time.count() / 1000000;
    auto usec = time.count() % 1000000;

    // set macro pulse number, buffer number and time stamp
    size_t ibuf = 0;
    if(_macroPulseNumberSource != nullptr) {
      ibuf = _macroPulseNumberSource->accessData(0) % nBuffers;
      macro_pulse(_macroPulseNumberSource->accessData(0), ibuf);
    }
    set_tmstmp(sec, usec, ibuf);

    if(_processArray->dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    // fill the spectrum
    std::vector<float>& processVector = _processArray->accessChannel(0);
    if(nBuffers == 1) {
      // We have to fill the spectrum differently if it is unbuffered, as the internal data structures seem to be
      // completely different.
      memcpy(spectrum()->d_spect_array.d_spect_array_val, processVector.data(), processVector.size() * sizeof(float));
      spectrum()->d_spect_array.d_spect_array_len = processVector.size();
    }
    else {
      fill_spectrum(processVector.data(), processVector.size(), ibuf);
    }
    if(publishZMQ) {
      dmsg_info info;
      memset(&info, 0, sizeof(info));
      info.sec = sec;
      info.usec = usec;
      if(_macroPulseNumberSource != nullptr) {
        info.ident = _macroPulseNumberSource->accessData(0);
      }
      else {
        info.ident = 0;
      }
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

  void DoocsSpectrum::setMacroPulseNumberSource(
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
    if(_processArray->isReadable()) {
      _macroPulseNumberSource = macroPulseNumberSource;
      if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
        _consistencyGroup.add(macroPulseNumberSource);
        _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct,
            std::bind(&DoocsSpectrum::updateDoocsBuffer, this, macroPulseNumberSource->getId()));
      }
    }
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
