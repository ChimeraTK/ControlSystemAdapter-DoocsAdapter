#include "DoocsAdapter.h"

namespace ChimeraTK{

  std::atomic<bool> DoocsAdapter::isInitialised(false);

  DoocsAdapter::DoocsAdapter(){
    // Create the managers. We need both
    std::pair<boost::shared_ptr<ControlSystemPVManager>,
	      boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();

    _controlSystemPVManager = pvManagers.first;
    _devicePVManager = pvManagers.second;

    updater = boost::make_shared<DoocsUpdater>();
  }

  boost::shared_ptr<DevicePVManager> const & DoocsAdapter::getDevicePVManager() const{
    return _devicePVManager;
  }

  boost::shared_ptr<ControlSystemPVManager> const & DoocsAdapter::getControlSystemPVManager() const{
    return _controlSystemPVManager;
  }

  void DoocsAdapter::waitUntilInitialised(){
    int i = 0;
    while(true){
      if (isInitialised){
        return;
      }
      // just sleep a bit. Use the "cheap" usleep, we don't care about precision here
      ++i;
      usleep(100);
    }
  }

}//namespace ChimeraTK
