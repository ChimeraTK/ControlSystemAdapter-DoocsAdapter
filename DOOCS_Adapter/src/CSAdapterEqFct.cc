#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"

namespace mtca4u{

  CSAdapterEqFct::CSAdapterEqFct(const char * fctName, int fctCode,
    boost::shared_ptr<ControlSystemPVManager> const & controlSystemPVManager)
   : EqFct (fctName),
     controlSystemPVManager_(controlSystemPVManager),
     fctCode_(fctCode){
    
    syncUtility_.reset (new mtca4u::ControlSystemSynchronizationUtility(controlSystemPVManager_));
    registerProcessVariablesInDoocs();
  }
  
  void CSAdapterEqFct::init(){
    std::cout << "this is eqfct init of " << fct_name() << std::endl;
  }

  void CSAdapterEqFct::update(){
    // Sending is done automatically when the "to device" variable is updated by Doocs.
    // No action needed here.
    // Call receive(), which triggers all receiveListeners and updates
    // the "deviceToControlSystem" variables, so Doocs knows the current values.
    // Only do this for the variables in this EqFct. All others have no callbacks in
    // this sync utility.
    syncUtility_->receive(mtca4uReceivers_);
  }
    
  int CSAdapterEqFct::fct_code(){
    return fctCode_;
  }



  void CSAdapterEqFct::registerProcessVariablesInDoocs(){
    // We only need the factory inside this function
    DoocsPVFactory factory(this, syncUtility_);
   
    // get all mtca4u process variables and reserve enough space for the same amount of doocs properties
    std::vector < mtca4u::ProcessVariable::SharedPtr > mtca4uProcessVariables =
      controlSystemPVManager_->getAllProcessVariables();
    // fixme: only take variables for this EqFct
    doocsProperties_.reserve( mtca4uProcessVariables.size() );

    // now create the doocs properties using the factory
    for( std::vector < mtca4u::ProcessVariable::SharedPtr >::iterator mtca4uVariableIter
	   = mtca4uProcessVariables.begin();
	 mtca4uVariableIter !=  mtca4uProcessVariables.end(); ++mtca4uVariableIter){
      doocsProperties_.push_back( factory.create( *mtca4uVariableIter ) );
      // we also have to remember which mtca4u variables we have to receive
      if ( (*mtca4uVariableIter)->isReceiver() ){
	mtca4uReceivers_.push_back(*mtca4uVariableIter);
      }
    }
}

}// namespace mtca4u
