#include "DoocsAdapter.h"

namespace ChimeraTK {

  std::atomic<bool> DoocsAdapter::isInitialised(false);

  DoocsAdapter::DoocsAdapter() {
    // Create the managers. We need both
    std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
        createPVManager();

    _controlSystemPVManager = pvManagers.first;
    _devicePVManager = pvManagers.second;

    updater = boost::make_shared<DoocsUpdater>();
  }

  boost::shared_ptr<DevicePVManager> const& DoocsAdapter::getDevicePVManager() const { return _devicePVManager; }

  boost::shared_ptr<ControlSystemPVManager> const& DoocsAdapter::getControlSystemPVManager() const {
    return _controlSystemPVManager;
  }

  void DoocsAdapter::waitUntilInitialised() {
    int i = 0;
    while(true) {
      if(isInitialised) {
        return;
      }
      // just sleep a bit. Use the "cheap" usleep, we don't care about precision
      // here
      ++i;
      usleep(100);
    }
  }

  bool DoocsAdapter::checkPrintDataLossWarning(size_t counter) {
    if(counter < 2) return false;
    if(counter == 2) return true;
    if(counter < 100) return counter % 10 == 0;
    if(counter < 1000) return counter % 100 == 0;
    if(counter < 10000) return counter % 1000 == 0;
    if(counter < 100000) return counter % 10000 == 0;
    if(counter < 1000000) return counter % 100000 == 0;
    return counter % 1000000 == 0; // if the rate is 10Hz, this is roughly once per day
  }

} // namespace ChimeraTK
