#include	"eq_cosade.h"
#include	"eq_sts_codes.h"
#include	"eq_fct_errors.h"

const char * object_name = "Cosade server";

CosadeServer::CosadeServer ( ) :
  EqFct ("NAME = cosade" ){
  // The DOOCSProcessVariableFactory is initialised with a 'this' of the EqFct we are just constructing.
  processVariableFactory_.reset( new mtca4u::DOOCSProcessVariableFactory(this) );

  // In Doocs you have to populate the factory in the server contructor.
  // Like this the DOOCS properties are known to the server and initialised
  // before the hardware initialisation (business logic) takes place in init().
  (void) processVariableFactory_->getProcessVariable<int> ("TARGET_VOLTAGE");
  (void) processVariableFactory_->getProcessVariable<int> ("MONITOR_VOLTAGE");
}

void CosadeServer::init ( )
{
  // Now that the variables are initialised, we can create an instance of the 
  // business logic, which initialises the hardware.
  // The control system independent 
  controlCore_.reset( new IndependentControlCore( processVariableFactory_ ) );
}

// used during startup of the server to create the locations
EqFct * eq_create (int eq_code, void *){
   switch (eq_code) {
      case CODE_COSADE:
	 return  new CosadeServer ();
      default:
	 return (EqFct *) 0;
   }
}

void CosadeServer::update(){
  // In the update function we call the control-system independent business logic.
  // This step will also be automated in future, so there will be one dedicated function to be called here.
  controlCore_->writeToHardware();
  controlCore_->readFromHardware();
}

// all the bloat we have to implement for DOOCS although we don't need it
void eq_init_prolog () {}
void eq_init_epilog () {}
void refresh_prolog () {}
void refresh_epilog () {}
void interrupt_usr1_prolog(int)  {}
void interrupt_usr2_prolog(void) {}
void interrupt_usr1_epilog(int)  {}
void interrupt_usr2_epilog(void) {}
void post_init_prolog(void)  	 {}
void post_init_epilog(void)	 {}
void eq_cancel(void)	 	 {}
