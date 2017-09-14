#ifndef __DOOCS_SPECTRUM_H__
#define __DOOCS_SPECTRUM_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include <mtca4u/NDRegisterAccessor.h>
#include "splitStringAtFirstSlash.h"

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace ChimeraTK {
  
  class DoocsSpectrum : public D_spectrum, public boost::noncopyable, public mtca4u::TransferElement {
        
    public:

      /** The constructor expects an NDRegisterAccessor of float, which usually will be a decorator
       *  to the implementation type. The decorator cannot be generated in the constructor
       *  because the ProcessVariable aka TransferElement does not know about it's size,
       *  which is needed by the D_spectrum constructor. This is not a big drawback because
       *  the properties are greated by a factory function anyway.
       */
      DoocsSpectrum( EqFct *eqFct, std::string const & doocsPropertyName,
                     boost::shared_ptr<  mtca4u::NDRegisterAccessor<float> > const &processArray)
        : D_spectrum( doocsPropertyName.c_str(),
                    processArray->getNumberOfSamples(), eqFct),
        _processArray( processArray )
      {}

      /**
       * Overload the set function which is called by DOOCS to inject sending to the device.
       */
      void set(EqAdr *eqAdr, EqData *data1, EqData *data2, EqFct *eqFct) {
        D_spectrum::set(eqAdr, data1, data2, eqFct);
        sendToDevice();
      }
      
      /**
       * Override the Doocs auto_init() method, which is called after initialising the value of
       *  the property from the config file.
       */
      void auto_init (void) {
        D_spectrum::auto_init();
        // send the current value to the device
        if (_processArray->isWriteable()){
          sendToDevice();
        }
      }

      // implement the stuff needed by TransferElement
      // FIXME: make this code reusable, just don't know how yet
      virtual const std::type_info& getValueType() const override{
        return typeid(float);
      }
      virtual TransferFuture& readAsync() override{
        return _processArray->readAsync();
      }
      virtual bool write(ChimeraTK::VersionNumber /*versionNumber*/={}) override{
        sendToDevice();
        // not checking for buffer overflow.
        //FIXEM: should this be writable at all?
        return false;
      }
      virtual void doReadTransfer() override{
        _processArray->doReadTransfer();
      }
      virtual bool doReadTransferNonBlocking() override{
        return _processArray->doReadTransferNonBlocking();        
      }
      virtual bool doReadTransferLatest() override{
        return _processArray->doReadTransferLatest();        
      }
      virtual void postRead(){
        _processArray->postRead();

       // FIXME: find the efficient memcopying implementation for float
        std::vector<float> & processVector = _processArray->accessChannel(0); 
        for(size_t i=0; i < processVector.size(); ++i) {
          fill_spectrum(i, processVector[i]);
        }
      }

      virtual bool isSameRegister(const boost::shared_ptr<TransferElement const> &other) const override{
        return _processArray->isSameRegister(other);
      }
      virtual bool isReadOnly() const override{
        //FIXME: We cannot access the doocs information here. 
        // because get_access is not const,
        // only provides an implementation dependent int without constant definitions anyway.
        // And d_access is private, not protected.
        return _processArray->isReadOnly();
      }
      virtual bool isWriteable() const override{
        return _processArray->isWriteable();
      }
      virtual bool isReadable() const override{
        return _processArray->isReadable();
      }
      virtual std::vector< boost::shared_ptr<TransferElement> > getHardwareAccessingElements() override{
        return _processArray->getHardwareAccessingElements();
      }
      virtual void replaceTransferElement(boost::shared_ptr<TransferElement> newElement){
        return _processArray->replaceTransferElement(newElement);
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

