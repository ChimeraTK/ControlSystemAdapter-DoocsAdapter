#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"
#include "splitStringAtFirstSlash.h"

namespace mtca4u{

  bool CSAdapterEqFct::emptyLocationVariablesHandled = false;
  

  CSAdapterEqFct::CSAdapterEqFct(int fctCode,
    boost::shared_ptr<ControlSystemPVManager> const & controlSystemPVManager,
    std::string fctName)
    // The second argument in EqFct has to be a pointer to string, and NULL pointer is
    // used when the name is coming from the config file. This interface is so ugly that
    // I changed it to std::string and need the ?: trick to get a NULL pointer in 
    // if the string is empty
    : EqFct ("NAME = CSAdapterEqFct", fctName.empty()?NULL:&fctName),
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

    auto processVariablesInThisLocation = getProcessVariablesInThisLocation();
    doocsProperties_.reserve( processVariablesInThisLocation.size() );

    // now create the doocs properties using the factory
    for( auto mtca4uVariable : processVariablesInThisLocation ){
      doocsProperties_.push_back( factory.create( mtca4uVariable ) );
      // we also have to remember which mtca4u variables we have to receive
      if ( mtca4uVariable->isReceiver() ){
	mtca4uReceivers_.push_back(mtca4uVariable);
      }
    }
  }

  std::vector < mtca4u::ProcessVariable::SharedPtr >
    CSAdapterEqFct::getProcessVariablesInThisLocation(){
    std::vector < mtca4u::ProcessVariable::SharedPtr > pvsInThisLocation;

    auto allPVs = controlSystemPVManager_->getAllProcessVariables();

    for (auto pv : allPVs){
      auto locationAndName = splitStringAtFirstSlash( pv->getName() );
      if ( locationAndName.first == fct_name() ){
	pvsInThisLocation.push_back( pv );
      }else if(locationAndName.first == "" && emptyLocationVariablesHandled == false) {
	pvsInThisLocation.push_back( pv );
      }
    }

    // the first location to run this function is getting the PVs with empty location name
    emptyLocationVariablesHandled = true;

    return pvsInThisLocation;
  }



}// namespace mtca4u