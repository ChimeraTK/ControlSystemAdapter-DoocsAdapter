#include "DoocsSpectrum.h"

#include <eq_fct.h>

#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

namespace ChimeraTK {

  DoocsSpectrum::DoocsSpectrum( EqFct *eqFct, std::string const & doocsPropertyName,
    boost::shared_ptr<  mtca4u::NDRegisterAccessor<float> > const &processArray,
    DoocsUpdater & updater,
    boost::shared_ptr<  mtca4u::NDRegisterAccessor<float> > const &startAccessor,
    boost::shared_ptr<  mtca4u::NDRegisterAccessor<float> > const &incrementAccessor)

    : D_spectrum( doocsPropertyName.c_str(), processArray->getNumberOfSamples(), eqFct),
      _processArray( processArray ), _startAccessor(startAccessor), _incrementAccessor(incrementAccessor)
  {
    if (processArray->isReadable()){
      updater.addVariable( ChimeraTK::OneDRegisterAccessor<float>(processArray) , std::bind(&DoocsSpectrum::updateDoocsBuffer, this));
    }
    if (startAccessor && startAccessor->isReadable()){
      updater.addVariable( ChimeraTK::ScalarRegisterAccessor<float>(startAccessor), std::bind(&DoocsSpectrum::updateParameters, this));
    }
    if (incrementAccessor && incrementAccessor->isReadable()){
      updater.addVariable( ChimeraTK::ScalarRegisterAccessor<float>(incrementAccessor), std::bind(&DoocsSpectrum::updateParameters, this));
    }
  }

  void DoocsSpectrum::set(EqAdr *eqAdr, EqData *data1, EqData *data2, EqFct *eqFct){
    D_spectrum::set(eqAdr, data1, data2, eqFct);
    sendToDevice();
  }

  void DoocsSpectrum::auto_init (void){
    D_spectrum::auto_init();
    // send the current value to the device
    if (_processArray->isWriteable()){
      sendToDevice();
    }
  }

  void DoocsSpectrum::updateDoocsBuffer(){
    // FIXME: find the efficient memcopying implementation for float
    std::vector<float> & processVector = _processArray->accessChannel(0);

    if (this->get_eqfct()){
      this->get_eqfct()->lock();
    }

    for(size_t i=0; i < processVector.size(); ++i) {
      fill_spectrum(i, processVector[i]);
    }

    if (this->get_eqfct()){
      this->get_eqfct()->unlock();
    }
  }

  void DoocsSpectrum::updateParameters(){
    if (this->get_eqfct()){
      this->get_eqfct()->lock();
    }

    float start, increment;
    if (_startAccessor){
      start=_startAccessor->accessData(0);
    }else{
      start=this->spec_start();
    }
    if (_incrementAccessor){
      increment=_incrementAccessor->accessData(0);
    }else{
      increment=this->spec_inc();
    }

    spectrum_parameter( this->spec_time(), start, increment, this->spec_status() );
    if (this->get_eqfct()){
      this->get_eqfct()->unlock();
    }
  }

  void DoocsSpectrum::sendToDevice() {
    // Brute force implementation with a loop. Works for all data types.
    // FIXME: find the efficient, memcopying function for float
    // always get a fresh reference
    std::vector<float> &processVector = _processArray->accessChannel(0);
    size_t arraySize = processVector.size();
    for (size_t i=0; i < arraySize; ++i){
      processVector[i] = read_spectrum(i);
    }
    _processArray->write();
  }

} // namespace ChimeraTK


