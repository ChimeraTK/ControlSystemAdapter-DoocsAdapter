#include "CosadeEqFct.h"

CosadeEqFct::CosadeEqFct ( ) : EqFct ("NAME = cosade" ), doocsAdapter_(this){
  
  // The DoocsAdapter is already prepared in initialiser list.
  // Now we can create our business logic and parse the DevicePVManager to it, which we get from the adapter.
  controlCore_.reset( new IndependentControlCore( doocsAdapter_.getDevicePVManager() ) );

  // Now that the business logic is created all ProcessVariables are known. We can tell the adapter
  // to create Doocs properties from them.
  doocsAdapter_.registerProcessVariablesInDoocs();

}

void CosadeEqFct::init ( ){
  /*empty in this example*/
}

void CosadeEqFct::update(){
  // Sending is done automatically when the "to device" variable is updated by Doocs. No action needed here.
  // Just call receiveAll(), which triggers all receiveListeners and updates the "deviceToControlSystem" 
  // variables, so Doocs knows the current values.
  doocsAdapter_.receiveAll();
}
