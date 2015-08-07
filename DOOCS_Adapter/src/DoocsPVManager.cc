#include "DoocsPVManager.h"

namespace mtca4u{

  DoocsPVManager::DoocsPVManager( boost::shared_ptr<ControlSystemPVManager> pvManager )
    : _pvManager(pvManager){
  }    

//    std::map< std::string, ControlSystemProcessVariable::SharedPtr > getAllProcessVariables () const;
  
  void DoocsPVManager::synchronize(){
    // Put all modified variables into the "from device" synchronisation list.
    // They all have to be updated if they have changed in the device.
    std::list< ControlSystemProcessVariable::SharedPtr > fromDeviceProcessVariables;
    while( ControlSystemProcessVariable::SharedPtr nextModification = _pvManager->nextNotification() ){
      fromDeviceProcessVariables.push_back( nextModification );
    }
    
    _pvManager->synchronize(_toDeviceProcessVariables, fromDeviceProcessVariables);
    _toDeviceProcessVariables.clear();
  }

  void DoocsPVManager::setModified(ControlSystemProcessVariable::SharedPtr & processVariable){
    _toDeviceProcessVariables.push_back( processVariable );
  }
  
}// namespace mtca4u
