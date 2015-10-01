#include "eq_cosade.h"

const char * object_name = "Cosade server";

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

// The usual function to create locations. Present in every doocs server.
EqFct * eq_create (int eq_code, void *){
   switch (eq_code) {
      case CODE_COSADE:
	 return  new CosadeEqFct ();
      default:
	 return (EqFct *) 0;
   }
}

// all the bloat we have to implement for DOOCS although we don't need it
void eq_init_prolog() {}
void eq_init_epilog() {}
void refresh_prolog() {}
void refresh_epilog() {}
void interrupt_usr1_prolog(int) {}
void interrupt_usr2_prolog(void) {}
void interrupt_usr1_epilog(int) {}
void interrupt_usr2_epilog(void) {}
void post_init_prolog(void) {}
void post_init_epilog(void) {}
void eq_cancel(void) {}
