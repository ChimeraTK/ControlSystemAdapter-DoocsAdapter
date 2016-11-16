#ifndef __DOOCS_PROCESS_SCALAR_H__
#define __DOOCS_PROCESS_SCALAR_H__

#include <string>
#include <ChimeraTK/ControlSystemAdapter/ProcessArray.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariableListener.h>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h>
#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>
#include "splitStringAtFirstSlash.h"

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
class DoocsProcessScalar :  public DOOCS_T{

protected:
  /** A helper class to register notifications for DoocsProcessScalars
   */
  class DoocsScalarListener: public ProcessVariableListener{
    public:
      /** The constructor gets a pointer to a Doocs variable. As it is only used inside a 
       *  DoocsProcessVariable, we do not have to care about the scope of the pointer. It 
       *  will always be valid.
       */
      DoocsScalarListener( DOOCS_T * doocsVariable ): _doocsVariable(doocsVariable){
      }
      
      /** The notification that is executed updates of the doocs process variable
       */
      void notify(boost::shared_ptr< ProcessVariable > processVariable){
	// It is safe to static cast because the DoocsScalarListener is inside a 
	// DoocsProcessScalar, which always holds the right type
        _doocsVariable->set_value( (static_cast< ProcessArray<T> & >(*processVariable)).accessData(0) );
      }

    private:
      DOOCS_T * _doocsVariable;
    };
 
    boost::shared_ptr< ProcessArray<T> > _processScalar;

public:
  DoocsProcessScalar( EqFct * const eqFct,
		      boost::shared_ptr< typename ChimeraTK::ProcessArray<T> > const & processScalar,
		      ControlSystemSynchronizationUtility & syncUtility)
    : DOOCS_T( splitStringAtFirstSlash(processScalar->getName()).second.c_str(), eqFct),
      _processScalar(processScalar) {
    syncUtility.addReceiveNotificationListener( processScalar->getName(),
      ProcessVariableListener::SharedPtr( new DoocsScalarListener(this) ) );
  }

  DoocsProcessScalar< T, DOOCS_T, DOOCS_VALUE_T > & operator= (DoocsProcessScalar< T, DOOCS_T, DOOCS_VALUE_T> const &other){
    // this uses the conversion operator to T when set_value is called
    set_value( other );
    return *this;
  }

  DoocsProcessScalar< T, DOOCS_T, DOOCS_VALUE_T > & operator= (T const &t){
    set_value(t);
    return *this;
  }

  /** Override the Doocs set_value method. This is called by all assignment operators
   *  and the DOOCS_T::set() method which is triggered by the RPC calls.
   *  For the overloading to work the signature has to be exactly as in DOOCS_T, so we need
   *  the value type (int for D_int, for instance). This is not necessarily the same as T.
   */
  void set_value(DOOCS_VALUE_T t){
    DOOCS_T::set_value(t);
    _processScalar->accessData(0) = t;
    if (_processScalar->isWriteable()){
      _processScalar->write();
    }
  }
  
  /** Override the Doocs auto_init() method, which is called after initialising the value of
   *  the property from the config file. */
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

