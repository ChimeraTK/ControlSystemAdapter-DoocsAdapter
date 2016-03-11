#ifndef CS_ADAPTER_EQ_FCT_H
#define CS_ADAPTER_EQ_FCT_H

#include <eq_fct.h>
#include <DoocsAdapter.h>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

template<class DeviceLogic_T>
class CSAdapterEqFct : public EqFct , boost::noncopyable {
 private:
  /* You need an instance of the mtca4u DoocsAdapter.
   */
  mtca4u::DoocsAdapter doocsAdapter_;

  /* You also need an instance of your business logic. It has to be allocated dynamically. 
   * We use smart pointers to avoid the hassle with memory management.
   */
  boost::scoped_ptr<DeviceLogic_T> deviceLogic_;

  
  int fctCode_;

 public:
  CSAdapterEqFct(const char * fctName, int fctCode)
    : EqFct (fctName), doocsAdapter_(this), fctCode_(fctCode){
    // The DoocsAdapter is already prepared in initialiser list.
    // Now we can create our business logic and parse the DevicePVManager to it,
    // which we get from the adapter.
    deviceLogic_.reset( new DeviceLogic_T( doocsAdapter_.getDevicePVManager() ) );

    // Now that the business logic is created all ProcessVariables are known.
    // We can tell the adapter to create Doocs properties from them.
    doocsAdapter_.registerProcessVariablesInDoocs();
  }
  
  void init(){
    std::cout << "this is eqfct init" << std::endl;
  }

  void update(){
    // Sending is done automatically when the "to device" variable is updated by Doocs.
    // No action needed here.
    // Just call receiveAll(), which triggers all receiveListeners and updates
    // the "deviceToControlSystem" variables, so Doocs knows the current values.
    doocsAdapter_.receiveAll();
  }
    
  int fct_code(){
    return fctCode_;
  }

};

#endif// CS_ADAPTER_EQ_FCT_H
