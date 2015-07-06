#ifndef __DOOCS_PROCESS_SCALAR_H__
#define __DOOCS_PROCESS_SCALAR_H__


#include <string>
#include <ControlSystemAdapter/ControlSystemProcessScalar.h>
#include <ControlSystemAdapter/ProcessScalar.h>
#include "DoocsPVManager.h"
//#include <boost/bind.hpp>
//#include <boost/function.hpp>

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace mtca4u {

template <typename T, typename DOOCS_T>
class DoocsProcessScalar :  public ControlSystemProcessScalar< T >, public DOOCS_T{

private:
  // only a weak pointer can be stored, because the manager is holding the instances of this class
  boost::weak_ptr<DoocsPVManager> _pvManager;

public:
  DoocsProcessScalar (std::string const & name, EqFct *eqFct, boost::shared_ptr<DoocsPVManager> doocsPVManager)
    : ProcessVariable(name), DOOCS_T(name.c_str(), eqFct), _pvManager(doocsPVManager) {
  }

  //  ~DoocsProcessScalar{}

  DoocsProcessScalar< T, DOOCS_T > & operator= (DoocsProcessScalar< T, DOOCS_T> const &other){
    set_value( other.value() );
    return *this;
  }

  ControlSystemProcessScalar< T > & operator= (ProcessScalar< T > const &other){
    set_value(other);
    return *this;
  }

  ControlSystemProcessScalar< T > & operator= (T const &t){
    set_value(t);
    return *this;
  }

  void set(DoocsProcessScalar< T, DOOCS_T > const &other){
    set_value(other.value());
  }

  void set(ProcessScalar< T > const &other){
    set_value(other.get());
  }

  void set (T const &t){
    set_value(t);
  }

  operator T() const{
    return DOOCS_T::value_;
  }

  T get() const{
    return DOOCS_T::value_;
  }

  /** Override the Doocs set_value method. This is called by all other setters, incl. the assignment operators
   *  and the DOOCS_T::set() method which is triggered by the RPC calls.
   */
  void set_value(T const &t){
    DOOCS_T::set_value(t);

    // Get the shared version of "this". It is needed to give it to the manager to put it into the queue.
    // FIXME: Store it as a weak pointer?
    boost::shared_ptr< DoocsPVManager> manager = _pvManager.lock();
    typename ControlSystemProcessVariable::SharedPtr sharedThis = manager->getProcessScalar<T>(this->_name);
    manager->setModified( sharedThis );
  }

  private:
  virtual void synchronizeToDevice() {
    typename DeviceProcessScalar<T>::SharedPtr peer =
      ControlSystemProcessScalar<T>::getPeer();
    if (peer) {
      T oldValue = peer->get();
      peer->set(*this);
      peer->triggerOnSetCallbackFunction(oldValue);
    }
  }
  
  virtual void synchronizeFromDevice() {
    typename DeviceProcessScalar<T>::SharedPtr peer =
      ControlSystemProcessScalar<T>::getPeer();
    if (peer) {
      peer->triggerOnGetCallbackFunction();
      // Directly set the doocs content without triggering the "to device" notification.
      DOOCS_T::set_value(peer->get());
    }
  }


// DOOCS standard interface override for CS-side requests

// nothing to override for get (?). The callback is handled asynchronously on the device side at the moment
//  void get(EqAdr * addr, EqData * eqData1, EqData * eqData2, EqFct * eqFct){
//    DOOCS_T::get (addr, eqData1, eqData2, eqFct);
//  }

// nothing to override here
//  void    set (EqAdr * eqAddr, EqData * eqData1, EqData * eqData2, EqFct * eqFct) {
//    // set the new value of the variable (the original doocs call)
//    DOOCS_T::set (eqAddr, eqData1, eqData2, eqFct);
//    
//    // Get the shared version of "this". It is needed to give it to the manager to put it into the queue.
//    // FIXME: Store it as a weak pointer?
//    boost::shared_ptr< DoocsPVManager> manager = _pvManager.lock();
//    SharedPtr sharedThis = _pvManager->(_name);
//    _pvManager->setModified( sharedThis );
//  }
//};

//template <>
//class DoocsProcessScalar <std::string, D_string> : public D_string
//{
//protected:
//	boost::function < void (std::string const &, std::string const & ) >	_onSetCallbackFunction;	// (newValue, oldValue)
//	boost::function < std::string () >				            			_onGetCallbackFunction;
//    
//    
//public:
//    
//
//
//    void setOnSetCallbackFunction (boost::function < void (std::string const &, std::string const & ) > onSetCallbackFunction)
//         {
//             _onSetCallbackFunction = onSetCallbackFunction;
//         }
//
//
//    void setOnGetCallbackFunction (boost::function < std::string () > onGetCallbackFunction)
//         {
//             _onGetCallbackFunction = onGetCallbackFunction;
//         }
//
//
//    void clearOnSetCallbackFunction ()
//         {
//             _onSetCallbackFunction.clear ();
//         }
//
//
//    void clearOnGetCallbackFunction ()
//         {
//             _onGetCallbackFunction.clear ();
//         }
//
//
//
//
//    DoocsProcessScalar <std::string, D_string> (const char *pn, EqFct *ef) :    D_string(pn, ef)
//            {
//                D_string::set_value("<empty>");
//            }
//
//
//            // !!!: the getters/setters remain const char* -based, so as to retain compliance with the D_string interface   (FIXME: probably not needed anymore)
//            // accessors with callbacks (not based on standard DOOCS property interface - a custom extension), for BL-side requests
//	void	setval (const char * val)
//            {
//                // cache the current value in the DOOCS property
//                std::string oldVal = std::string(D_string::value());
//                // set the new value of the variable
//                D_string::set_value(val);
//                if (_onSetCallbackFunction)
//                {
//                    // trigger the callback with both the new and the old value
//                    _onSetCallbackFunction (std::string(val), oldVal);
//                }
//            }
//	const char *    getval ()
//            {
//                if (_onGetCallbackFunction)
//                {
//                    D_string::set_value( _onGetCallbackFunction().c_str() );
//                }
//                return D_string::value();
//            }
//
//
//            // accessors without callbacks (not based on standard DOOCS property interface - a custom extension), for BL-side requests
//	void	setval_without_callback (const char * val)
//            {
//                D_string::set_value(val);
//            }
//	const char * 	getval_without_callback ()
//            {
//                return D_string::value();
//            }
//
//
//            // DOOCS standard interface override for CS-side requests
//    void    get (EqAdr * _1, EqData * _2, EqData * _3, EqFct * _4)
//            {
//                if (_onGetCallbackFunction)
//                {
//                    D_string::set_value( _onGetCallbackFunction().c_str() );
//                }
//                D_string::get (_1, _2, _3, _4);
//            }
//    void    set (EqAdr * _1, EqData * _2, EqData * _3, EqFct * _4)
//            {
//                // cache the current value in the DOOCS property
//                std::string oldVal = std::string(D_string::value());
//                // set the new value of the variable
//                D_string::set (_1, _2, _3, _4);
//                // get the new value from the DOOCS property
//                std::string val = std::string(D_string::value());
//                if (_onSetCallbackFunction)
//                {
//                    // trigger the callback with both the new and the old value
//                    _onSetCallbackFunction (val, oldVal);
//                }
//            }
};



} // namespace mtca4u


#endif // __DOOCS_PROCESS_SCALAR_H__

