#ifndef __DOOCS_PROCESS_SCALAR_H__
#define __DOOCS_PROCESS_SCALAR_H__

#include <string>
#include <ChimeraTK/ControlSystemAdapter/ProcessArray.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariableListener.h>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h>
#include <boost/shared_ptr.hpp>
#include <d_fct.h>

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace ChimeraTK {

  /** The DoocsProcessScalar has three template parameters:
  *  \li \c T, The primitive value type of the ChimeraTK process variable
  *  \li \c DOOCS_T, The Doocs type which is used
  *  \li \c DOOCS_VALUE_T, The primitive type of the Doocs type, which is not necessarily the same as T
  * 
  *  The last type is necessary for the set_value overloading to work. If for instance T is short, then
  *  D_int is used and the signature is set_value(int), not set_value(short).
  */
  template <typename T, typename DOOCS_T, typename DOOCS_VALUE_T>
  class DoocsProcessScalar :  public DOOCS_T {

    protected:

      /**
      * A helper class to register notifications for DoocsProcessScalars
      */
      class DoocsScalarListener: public ProcessVariableListener {

        public:

          /**
          * The constructor gets a pointer to a Doocs variable. As it is only used inside a 
          * DoocsProcessVariable, we do not have to care about the scope of the pointer. It 
          * will always be valid.
          */
          DoocsScalarListener(DOOCS_T * doocsVariable)
          : _doocsVariable(doocsVariable)
          {}
          
          /**
          * The notification that is executed updates of the doocs process variable
          */
          void notify(boost::shared_ptr< ProcessVariable > processVariable) {
            // It is safe to static cast because the DoocsScalarListener is inside a 
            // DoocsProcessScalar, which always holds the right type
            auto data = (static_cast< ProcessArray<T> & >(*processVariable)).accessData(0);
            // we must not call set_and_archive if there is no history (otherwise it will be activated), but we have to if it is there. -> Abstraction, please!
            if (_doocsVariable->get_histPointer()){
              _doocsVariable->set_and_archive(data);
            }else{
              _doocsVariable->set_value(data);
            }
          }

        private:

          DOOCS_T * _doocsVariable;
      };
    
      boost::shared_ptr<ProcessArray<T>> _processScalar;

    private:

      // This class is neither assignable nor copy-constructible
      DoocsProcessScalar< T, DOOCS_T, DOOCS_VALUE_T > & operator= (DoocsProcessScalar< T, DOOCS_T, DOOCS_VALUE_T> const &other);
      DoocsProcessScalar(DoocsProcessScalar< T, DOOCS_T, DOOCS_VALUE_T> const &other);
      
    public:

      DoocsProcessScalar( EqFct *eqFct, std::string doocsPropertyName,
                          boost::shared_ptr< typename ChimeraTK::ProcessArray<T> > const &processScalar,
                          ControlSystemSynchronizationUtility &syncUtility )
      : DOOCS_T(eqFct, doocsPropertyName.c_str()), _processScalar(processScalar)
      {
        syncUtility.addReceiveNotificationListener( processScalar,
                                                    ProcessVariableListener::SharedPtr(new DoocsScalarListener(this)) );
      }

      DoocsProcessScalar( std::string doocsPropertyName, EqFct *eqFct,
                          boost::shared_ptr< typename ChimeraTK::ProcessArray<T> > const &processScalar,
                          ControlSystemSynchronizationUtility &syncUtility )
      : DOOCS_T(doocsPropertyName.c_str(), eqFct), _processScalar(processScalar)
      {
        syncUtility.addReceiveNotificationListener( processScalar,
                                                    ProcessVariableListener::SharedPtr(new DoocsScalarListener(this)) );
      }

      /**
      * Override the Doocs set method which is triggered by the RPC calls.
      */
      void set(EqAdr *adr, EqData *data1, EqData *data2, EqFct *eqfct) override{
        // only assign the value if the variable is writeable
        // Otherwise the content displayed by Doocs and the value in the application are inconsistent
        if (_processScalar->isWriteable()){
          DOOCS_T::set(adr, data1, data2, eqfct);
          // let the DOOCS_T set function do all the dirty work and use the get_value function afterwards to get the already assigned value
          _processScalar->accessData(0) = this->value();
          _processScalar->write();
        }
      }
      
      /**
      * Override the Doocs auto_init() method, which is called after initialising the value of
      * the property from the config file.
      */
      void auto_init (void) {
        DOOCS_T::auto_init();
        // send the current value to the device
        if (_processScalar->isWriteable()){
          _processScalar->accessData(0) = DOOCS_T::value();
          _processScalar->write();
        }
      }

  };

} // namespace ChimeraTK

#endif // __DOOCS_PROCESS_SCALAR_H__

