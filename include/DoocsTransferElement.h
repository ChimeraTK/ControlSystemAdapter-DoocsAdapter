#ifndef __DOOCS_TRANSFER_ELEMENT_H__
#define __DOOCS_TRANSFER_ELEMENT_H__

#include <ChimeraTK/TransferElement.h>

namespace ChimeraTK {

  template<class UserType>
  class DoocsTransferElement : public ChimeraTK::TransferElement {
        
    public:

      /** The constructor expects an NDRegisterAccessor of UserType, which usually will be a decorator
       *  to the implementation type. The decorator cannot be generated in the constructor
       *  because the ProcessVariable aka TransferElement does not know about it's size,
       *  which is needed by the D_spectrum constructor. This is not a big drawback because
       *  the properties are greated by a factory function anyway.
       */
      DoocsTransferElement( boost::shared_ptr<  ChimeraTK::NDRegisterAccessor<UserType> > const &processArray)
        : _processArray( processArray )
      {}


      // implement the stuff needed by TransferElement
      virtual const std::type_info& getValueType() const override{
        return typeid(UserType);
      }
      virtual TransferFuture& readAsync() override{
        return _processArray->readAsync();
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
      virtual void replaceTransferElement(boost::shared_ptr<TransferElement> newElement) override{
        return _processArray->replaceTransferElement(newElement);
      }

      
  protected:

      boost::shared_ptr< ChimeraTK::NDRegisterAccessor<UserType> > _processArray;
  };

} // namespace ChimeraTK

#endif // __DOOCS_TRANSFER_ELEMENT_H__

