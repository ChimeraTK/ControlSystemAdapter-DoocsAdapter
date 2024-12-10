// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsSpectrum.h"

#include "DoocsAdapter.h"
#include <eq_fct.h>

#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

#include <iostream>
#include <utility>

namespace ChimeraTK {

  DoocsSpectrum::DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> startAccessor,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> incrementAccessor)
  : D_spectrum(doocsPropertyName, processArray->getNumberOfSamples(), eqFct, processArray->isWriteable()),
    PropertyBase(doocsPropertyName, updater), _processArray(processArray), _startAccessor(std::move(startAccessor)),
    _incrementAccessor(std::move(incrementAccessor)), _nBuffers(1) {
    setupOutputVar(processArray);

    addParameterAccessors();
  }

  DoocsSpectrum::DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> startAccessor,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> incrementAccessor, size_t numberOfBuffers)
  : D_spectrum(doocsPropertyName, processArray->getNumberOfSamples(), eqFct, numberOfBuffers, DATA_A_FLOAT),
    PropertyBase(doocsPropertyName, updater), _processArray(processArray), _startAccessor(std::move(startAccessor)),
    _incrementAccessor(std::move(incrementAccessor)), _nBuffers(numberOfBuffers) {
    if(_nBuffers > 1 && !processArray->isReadable()) {
      throw ChimeraTK::logic_error(
          "D_spectrum '" + _processArray->getName() + "' has numberOfBuffers > 1 but is not readable.");
    }
    setupOutputVar(processArray);

    addParameterAccessors();
  }

  void DoocsSpectrum::set(EqAdr* eqAdr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqFct) {
    D_spectrum::set(eqAdr, data1, data2, eqFct);
    if(_macroPulseNumberSource != nullptr) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    modified = true;
    sendToDevice(true);

    sendZMQ(getTimestamp());
  }

  void DoocsSpectrum::auto_init() {
    doocsAdapter.beforeAutoInit();

    // check if the macro pulse number source has been set if the spectrum is buffered
    if(_nBuffers > 1) {
      if(_macroPulseNumberSource == nullptr) {
        throw ChimeraTK::logic_error(
            "D_spectrum '" + _processArray->getName() + "' has numberOfBuffers > 1 but not macro pulse number source.");
      }
    }

    // send the current value to the device
    D_spectrum::read();
    modified = false;
    if(this->get_access() == 1 ||
        (_processArray->isWriteable() && otherPropertiesToUpdate.empty())) { // property is writeable
      sendToDevice(false);
      // set DOOCS time stamp, workaround for DOOCS bug (get() always gives current time stamp if no timestamp is set,
      // which breaks consistency check in ZeroMQ subscriptions after the 4 minutes timeout)
      D_spectrum::set_stamp();
    }
  }

  void DoocsSpectrum::write(std::ostream& s) {
    // DOOCS is normally keeping the location lock until everything is written for that location: all D_spectrum and all
    // other properties. This can take too long (like seconds), which leads to noticable freezes of the UI. As a
    // work-around we release the lock here, wait some time and acquire the lock again. Since this happens in a separate
    // thread (svr_writer), this slow-down should not be of any harm.
    get_eqfct()->unlock();
    usleep(1000);
    get_eqfct()->lock();
    if(!modified || _processArray->isReadOnly()) {
      return;
    }
    modified = false;
    D_spectrum::write(s);
  }

  void DoocsSpectrum::addParameterAccessors() {
    if(_startAccessor && _startAccessor->isReadable()) {
      _doocsUpdater.addVariable(
          ChimeraTK::ScalarRegisterAccessor<float>(_startAccessor), getEqFct(), [this] { return updateParameters(); });
    }
    if(_incrementAccessor && _incrementAccessor->isReadable()) {
      _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<float>(_incrementAccessor), getEqFct(),
          [this] { return updateParameters(); });
    }
  }

  void DoocsSpectrum::updateDoocsBuffer(const TransferElementID& transferElementId) {
    // Note: we already own the location lock by specification of the DoocsUpdater

    // There are only the processArray and the macro pulse number in the consistency
    // group. The limits are coming asynchronously and not for every macro pulse,
    // so we just take test latest we have.
    if(!updateConsistency(transferElementId)) {
      return;
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
    auto seconds = sinceEpoch.seconds;
    auto microseconds = sinceEpoch.microseconds;

    // set macro pulse number, buffer number and time stamp
    size_t ibuf = 0;
    if(_macroPulseNumberSource != nullptr) {
      ibuf = _macroPulseNumberSource->accessData(0) % _nBuffers;
      macro_pulse(_macroPulseNumberSource->accessData(0), ibuf);
    }
    set_tmstmp(seconds, microseconds, ibuf);

    if(_processArray->dataValidity() != ChimeraTK::DataValidity::ok) {
      // this will set error code correctly in given buffer _and_ in main property attribute, while
      // d_error() sets it only in latter
      this->error(stale_data, ibuf);
    }
    else {
      this->error(no_error, ibuf);
    }

    // fill the spectrum
    std::vector<float>& processVector = _processArray->accessChannel(0);
    if(_nBuffers == 1) {
      // We have to fill the spectrum differently if it is unbuffered, as the internal data structures seem to be
      // completely different.
      memcpy(spectrum()->d_spect_array.d_spect_array_val, processVector.data(), processVector.size() * sizeof(float));
      spectrum()->d_spect_array.d_spect_array_len = processVector.size();
    }
    else {
      fill_spectrum(processVector.data(), processVector.size(), ibuf);
    }

    // mark property as modified, for (optional) persistence
    modified = true;

    sendZMQ(timestamp);
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

    // WORKAROUND: spectrum_parameter modifies the internal timestamp of
    // the spectrum, this confuses our code that it thinks it has already sent off the
    // spectrum with this timestamp, bumps the timestamp to enable DOOCS to check for inconsistencies
    // which then confuses the DAQ since it sees data for the same MP with different timestamps
    // https://mcs-gitlab.desy.de/doocs/doocs-core-libraries/serverlib/-/issues/35
    auto oldTimeStamp = this->get_timestamp();
    spectrum_parameter(this->spec_time(), start, increment, this->spec_status());
    this->set_timestamp(oldTimeStamp);
  }

  void DoocsSpectrum::sendToDevice(bool getLock) {
    sendArrayToDevice(this, _processArray);
    updateOthers(getLock);
  }

} // namespace ChimeraTK
