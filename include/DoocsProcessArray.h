#ifndef __DOOCS_PROCESS_ARRAY_H__
#define __DOOCS_PROCESS_ARRAY_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include <mtca4u/NDRegisterAccessor.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariableListener.h>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h>

#include "splitStringAtFirstSlash.h"

#include <eq_fct.h>

namespace ChimeraTK {
  
  template <typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  class DoocsProcessArray : public DOOCS_T, public boost::noncopyable {
        
  public:
      DoocsProcessArray( EqFct *eqFct, std::string const & doocsPropertyName,
                         boost::shared_ptr<  mtca4u::NDRegisterAccessor<DOOCS_PRIMITIVE_T> > const &processArray,
                         DoocsUpdater & updater)
        : DOOCS_T( doocsPropertyName.c_str(), processArray->getNumberOfSamples(), eqFct),
          _processArray( processArray )
      {
        if (processArray->isReadable()){
          updater.addVariable( *processArray , std::bind(&DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>::updateDoocsBuffer, this));
        }
      }

    /**
     * Overload the set function which is called by DOOCS to inject sending to the device.
     */
    void set(EqAdr *eqAdr, EqData *data1, EqData *data2, EqFct *eqFct) override{
      DOOCS_T::set(eqAdr, data1, data2, eqFct);
      sendToDevice();
    }
    
    /**
     * Override the Doocs auto_init() method, which is called after initialising the value of
     *  the property from the config file.
     */
    void auto_init (void) override{
      DOOCS_T::auto_init();
      // send the current value to the device
      if (_processArray->isWriteable()){
        sendToDevice();
      }
    }

    void  updateDoocsBuffer(){
      auto & processVector = _processArray->accessChannel(0);
      // fixme: what about history?

      // This function is called from some thread, so we have to use the location lock here
      // Process scalars can also be created with a NULL EqFct for testing, so we have to cath
      // this here. Obviously we can ignore the lock if there is no EqFct to access the content.
      if (this->get_eqfct()){
        this->get_eqfct()->lock();
      }
      this->fill_array(processVector.data(), processVector.size());
      if (this->get_eqfct()){
        this->get_eqfct()->unlock();
      }
    }
    
  protected:
    boost::shared_ptr< mtca4u::NDRegisterAccessor<DOOCS_PRIMITIVE_T> > _processArray;

    // Internal function which copies the content from the DOOCS container into the 
    // ChimeraTK ProcessArray and calls the send method. Factored out to allow unit testing.
    void sendToDevice() {
      // Brute force implementation with a loop. Works for all data types.
      // always get a fresh reference
      auto & processVector = _processArray->accessChannel(0); 
      size_t arraySize = processVector.size();
      for (size_t i=0; i < arraySize; ++i){
        processVector[i] = this->value(i);
      }
      _processArray->write();
    }
    
  };

} // namespace ChimeraTK

#endif // __DOOCS_PROCESS_ARRAY_H__

