#ifndef __DOOCS_SPECTRUM_H__
#define __DOOCS_SPECTRUM_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include <mtca4u/NDRegisterAccessor.h>
#include "DoocsUpdater.h"
#include "splitStringAtFirstSlash.h"

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace ChimeraTK {
  
  class DoocsSpectrum : public D_spectrum, public boost::noncopyable {
        
    public:

      /** The constructor expects an NDRegisterAccessor of float, which usually will be a decorator
       *  to the implementation type. The decorator cannot be generated in the constructor
       *  because the ProcessVariable aka TransferElement does not know about it's size,
       *  which is needed by the D_spectrum constructor. This is not a big drawback because
       *  the properties are greated by a factory function anyway.
       */
      DoocsSpectrum( EqFct *eqFct, std::string const & doocsPropertyName,
                     boost::shared_ptr<  mtca4u::NDRegisterAccessor<float> > const &processArray,
                     DoocsUpdater & updater)
        : D_spectrum( doocsPropertyName.c_str(), processArray->getNumberOfSamples(), eqFct),
          _processArray( processArray )
      {
        if (processArray->isReadable()){
          updater.addVariable( *processArray , std::bind(&DoocsSpectrum::updateDoocsBuffer, this));
        }
      }

      /**
       * Overload the set function which is called by DOOCS to inject sending to the device.
       */
      void set(EqAdr *eqAdr, EqData *data1, EqData *data2, EqFct *eqFct) override{
        D_spectrum::set(eqAdr, data1, data2, eqFct);
        sendToDevice();
      }
      
      /**
       * Override the Doocs auto_init() method, which is called after initialising the value of
       *  the property from the config file.
       */
      void auto_init (void) override{
        D_spectrum::auto_init();
        // send the current value to the device
        if (_processArray->isWriteable()){
          sendToDevice();
        }
      }

      // call this function after a tranfer element has requested it.
      void updateDoocsBuffer(){
       // FIXME: find the efficient memcopying implementation for float
        std::vector<float> & processVector = _processArray->accessChannel(0); 
        for(size_t i=0; i < processVector.size(); ++i) {
          fill_spectrum(i, processVector[i]);
        }
      }

  protected:
      boost::shared_ptr< mtca4u::NDRegisterAccessor<float> > _processArray;

      // Internal function which copies the content from the DOOCS container into the 
      // ChimeraTK ProcessArray and calls the send method. Factored out to allow unit testing.
      void sendToDevice() {
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

  };

} // namespace ChimeraTK

#endif // __DOOCS_SPECTRUM_H__

