#include "DoocsAdapter.h"

namespace ChimeraTK{

  DoocsAdapter::DoocsAdapter(){
    // Create the managers. We need both
    std::pair<boost::shared_ptr<ControlSystemPVManager>,
	      boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();

    _controlSystemPVManager = pvManagers.first;
    _devicePVManager = pvManagers.second;
  }

  boost::shared_ptr<DevicePVManager> const & DoocsAdapter::getDevicePVManager() const{
    return _devicePVManager;
  }

  boost::shared_ptr<ControlSystemPVManager> const & DoocsAdapter::getControlSystemPVManager() const{
    return _controlSystemPVManager;
  }

}//namespace ChimeraTK
