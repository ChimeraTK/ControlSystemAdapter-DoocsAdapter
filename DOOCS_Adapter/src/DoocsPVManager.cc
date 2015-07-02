#include "DoocsPVManager.h"

namespace mtca4u{

  DoocsPVManager::DoocsPVManager( boost::shared_ptr<ControlSystemPVManager> pvManager )
    : _pvManager(pvManager){
    
    // Put all variables into the "from device" synchronisation list. They all have to be updated 
    // if they have changed in the device.
    std::map< std::string, ControlSystemProcessVariable::SharedPtr > allVariablesMap
      = pvManager->getAllProcessVariables();
    for (std::map< std::string, ControlSystemProcessVariable::SharedPtr >::const_iterator it = 
	     allVariablesMap.begin(); it != allVariablesMap.end(); ++it){
      _fromDeviceProcessVariables.push_back( it->second );
    }
  }

//    std::map< std::string, ControlSystemProcessVariable::SharedPtr > getAllProcessVariables () const;
  
  void DoocsPVManager::synchronize(){
    _pvManager->synchronize(_toDeviceProcessVariables, _fromDeviceProcessVariables);
  }

  void DoocsPVManager::setModified(ControlSystemProcessVariable::SharedPtr & processVariable){
    _toDeviceProcessVariables.push_back( processVariable );
  }
  
}// namespace mtca4u
