#include "eq_cosade.h"
//#include	"eq_sts_codes.h"
//#include	"eq_fct_errors.h"
#include <DoocsPVFactory.h>

using namespace mtca4u;

const char * object_name = "Cosade server";

CosadeEqFct::CosadeEqFct ( ) : EqFct ("NAME = cosade" ){

  // create the managers. We need both
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();

  // The CS manager is needed in the EqFct (obviously this is the CS side),
  // so we store it in a class-wide variable
  processVariableManager_ = pvManagers.first;

  // The device manager is only needed to give it to the business logic
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  // now create an instance of the business logic. This creates all variables in
  // the managers.
  controlCore_.reset( new IndependentControlCore( devManager ) );
  
  syncUtility_.reset( 
    new mtca4u::ControlSystemSynchronizationUtility(processVariableManager_) );

  DoocsPVFactory factory(this, syncUtility_);

  // Take all variables and make them known to DOOCS, using the factory.
  // The variables are stored in a vector.
  std::vector < mtca4u::ProcessVariable::SharedPtr > mtca4uProcessVariables =
    processVariableManager_->getAllProcessVariables();

  processVariables_.reserve( mtca4uProcessVariables.size() );
  for( std::vector < mtca4u::ProcessVariable::SharedPtr >::iterator mtca4uVariableIter
	 = mtca4uProcessVariables.begin();
       mtca4uVariableIter !=  mtca4uProcessVariables.end(); ++mtca4uVariableIter){
    processVariables_.push_back( factory.create( *mtca4uVariableIter ) );
  }
}

void CosadeEqFct::init ( ){
  /*empty in this example*/
}

void CosadeEqFct::update(){
  syncUtility_->sendAll();
  // ok, this does not work as intended. equivalent to a 150 ms sleep, but with 
  // many unneeded wake-ups
  syncUtility_->waitForNotifications(150000,10000);
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
