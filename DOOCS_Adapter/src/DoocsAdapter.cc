#include "DoocsAdapter.h"

namespace mtca4u{

  DoocsAdapter::DoocsAdapter(EqFct *eqFct): _eqFct(eqFct){
    // Create the managers. We need both
    std::pair<boost::shared_ptr<ControlSystemPVManager>,
	      boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();

    _controlSystemPVManager = pvManagers.first;
    _devicePVManager = pvManagers.second;

    // Finally store the SyncUtility for the csManager
    _syncUtility.reset( new mtca4u::ControlSystemSynchronizationUtility(_controlSystemPVManager) );
  }

  boost::shared_ptr<DevicePVManager> & DoocsAdapter::getDevicePVManager(){
    return _devicePVManager;
  }

  void DoocsAdapter::registerProcessVariablesInDoocs(){
    // We only need the factory inside this function
    DoocsPVFactory factory(_eqFct, _syncUtility);
   
    // get all mtca4u process variables and reserve enough space for the same amount of doocs properties
    std::vector < mtca4u::ProcessVariable::SharedPtr > mtca4uProcessVariables =
      _controlSystemPVManager->getAllProcessVariables();
    _doocsProperties.reserve( mtca4uProcessVariables.size() );

    // not create the doocs properties using the factory
    for( std::vector < mtca4u::ProcessVariable::SharedPtr >::iterator mtca4uVariableIter
	   = mtca4uProcessVariables.begin();
	 mtca4uVariableIter !=  mtca4uProcessVariables.end(); ++mtca4uVariableIter){
      _doocsProperties.push_back( factory.create( *mtca4uVariableIter ) );
    }
  }

  void DoocsAdapter::receiveAll(){
    _syncUtility->receiveAll();
  }

}//namespace mtca4u
