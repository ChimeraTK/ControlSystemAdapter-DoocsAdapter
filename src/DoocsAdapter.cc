// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsAdapter.h"

#include "DoocsUpdater.h"

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

  boost::shared_ptr<DevicePVManager> const& DoocsAdapter::getDevicePVManager() const {
    return _devicePVManager;
  }

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
    // print first time at counter == 10 to suppress the messages spamming at startup
    if(counter < 1) return false;
    if(counter < 100) return counter % 10 == 0;
    if(counter < 1000) return counter % 100 == 0;
    if(counter < 10000) return counter % 1000 == 0;
    if(counter < 100000) return counter % 10000 == 0;
    if(counter < 1000000) return counter % 100000 == 0;
    return counter % 1000000 == 0; // if the rate is 10Hz, this is roughly once per day
  }

  void DoocsAdapter::before_auto_init() {
    // prevent concurrent execution. It is unclear whether DOOCS may call auto_init in parallel in some situations, so
    // better implement a lock.
    static std::mutex mx;
    std::unique_lock<std::mutex> lk(mx);

    // execute actions only once
    if(before_auto_init_called) return;
    before_auto_init_called = true;

    // connect properties which use the same writable PV to keep the property values consistent
    for(auto& group : doocsAdapter.writeableVariablesWithMultipleProperties) {
      for(const auto& property : group.second) {
        auto pc = boost::dynamic_pointer_cast<D_fct>(property);
        property->otherPropertiesToUpdate = group.second;
        property->otherPropertiesToUpdate.erase(property); // the property should not be in its own list
      }
    }
    doocsAdapter.writeableVariablesWithMultipleProperties.clear(); // save memory (information no longer needed)
  }

} // namespace ChimeraTK
