#ifndef __DOOCS_PROCESS_SCALAR_H__
#define __DOOCS_PROCESS_SCALAR_H__

#include <string>
#include <ControlSystemAdapter/ProcessScalar.h>
#include <ControlSystemAdapter/ProcessVariableListener.h>
#include <ControlSystemAdapter/ControlSystemSynchronizationUtility.h>
#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace mtca4u {

template <typename T, typename DOOCS_T>
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
        std::cout << "This is DoocsScalarListener::DoocsScalarListener" << std::endl;
      }
      
      /** The notification that is executed updates of the doocs process variable
       */
      void notify(boost::shared_ptr< ProcessVariable > processVariable){
	// It is safe to static cast because the DoocsScalarListener is inside a 
	// DoocsProcessScalar, which always holds a ProcessScalar, never a ProcessArray
	std::cout << "This is DoocsScalarListener::notify() on "  
		  << processVariable->getName() << std::endl;
	_doocsVariable->set_value( static_cast< ProcessScalar<T> & >(*processVariable) );
      }

    private:
      DOOCS_T * _doocsVariable;
    };
 
  boost::shared_ptr< ProcessScalar<T> > _processScalar;

public:
  DoocsProcessScalar( EqFct *eqFct, boost::shared_ptr< typename mtca4u::ProcessScalar<T> > & processScalar,
		      ControlSystemSynchronizationUtility & syncUtility)
    : DOOCS_T(processScalar->getName().c_str(), eqFct),  _processScalar(processScalar) {
    syncUtility.addReceiveNotificationListener( processScalar->getName(),
      ProcessVariableListener::SharedPtr( new DoocsScalarListener(this) ) );
  }

  DoocsProcessScalar< T, DOOCS_T > & operator= (DoocsProcessScalar< T, DOOCS_T> const &other){
    // this uses the conversion operator to T when set_value is called
    set_value( other );
    return *this;
  }

  DoocsProcessScalar< T, DOOCS_T > & operator= (T const &t){
    set_value(t);
    return *this;
  }

  DOOCS_T & operator= (DOOCS_T const &other){
    set_value(other.value_);
    return *this;
  }

  /** Override the Doocs set_value method. This is called by all assignment operators
   *  and the DOOCS_T::set() method which is triggered by the RPC calls.
   */
  void set_value(T const &t){
    DOOCS_T::set_value(t);
    *_processScalar = t;
    if (_processScalar->isSender()){
      _processScalar->send();
    }
  }

  
};



} // namespace mtca4u


#endif // __DOOCS_PROCESS_SCALAR_H__

