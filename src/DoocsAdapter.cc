// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

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

  /************************************************************************/

  void PropertyBase::registerVariable(const TransferElementAbstractor& var) {
    if(var.isReadable()) {
      _doocsUpdater.addVariable(var, _eqFct, std::bind(&PropertyBase::updateDoocsBuffer, this, var.getId()));
      _consistencyGroup.add(var);
    }
  }

  bool PropertyBase::updateConsistency(const TransferElementID& updatedId) {
    // FIXME: A first  implementation is checking the data consistency here. Later this should be
    // before calling this function because calling this function through a function pointer is
    // comparatively expensive.

    // Do not check if update is coming from another DOOCS property mapped to the same variable (ID invalid), since
    // the check would never pass. Such variables cannot use exact data matching anyway, since the update is triggered
    // from the DOOCS write to the other property.
    // Also do not check, if data matching turned off
    if(!updatedId.isValid() || _consistencyGroup.getMatchingMode() == DataConsistencyGroup::MatchingMode::none) {
      _doocsSuccessfullyUpdated = true;
      return true;
    }
    assert(_mainOutputVar);
    TransferElementID compareTo = _mainOutputVar->getId();

    if(!_consistencyGroup.update(updatedId)) {
      // data is not consistent (yet). Don't update the Doocs buffer.
      // check if this will now throw away data and generate a warning
      if(updatedId == compareTo) {
        if(!_doocsSuccessfullyUpdated) {
          ++_nDataLossWarnings;
          if(DoocsAdapter::checkPrintDataLossWarning(_nDataLossWarnings)) {
            const char* reason = (bool)(_macroPulseNumberSource) ?
                " due to failed data matching between value and macro pulse number" :
                " due to failed data matching between values";

            std::cout << "WARNING: Data loss in property " << _eqFct->name() << "/" << _doocsPropertyName << reason
                      << " (repeated " << _nDataLossWarnings << " times)." << std::endl;
          }
        }
      }
      _doocsSuccessfullyUpdated = false;
      return false;
    }
    _doocsSuccessfullyUpdated = true;
    return true;
  }

  doocs::Timestamp PropertyBase::getTimestamp() {
    assert(_mainOutputVar);
    doocs::Timestamp timestamp(_mainOutputVar->getVersionNumber().getTime());
    return timestamp;
  }

  doocs::Timestamp PropertyBase::correctDoocsTimestamp() {
    doocs::Timestamp timestamp = getTimestamp();
    auto d_fct = getDfct();
    // Make sure we never send out two absolute identical time stamps. If we would do so, the "watchdog" which
    // corrects inconsistencies in ZeroMQ subscriptions between sender and subcriber cannot detect the inconsistency.
    if(d_fct->get_timestamp() == timestamp) {
      timestamp += std::chrono::microseconds(1);
    }

    // update global time stamp of DOOCS, but only if our time stamp is newer
    if(get_global_timestamp() < timestamp) {
      set_global_timestamp(timestamp);
    }
    d_fct->set_timestamp(timestamp);
    return timestamp;
  }

  void PropertyBase::sendZMQ(doocs::Timestamp timestamp) {
    auto d_fct = getDfct();
    // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    if(_publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
      dmsg_info info;
      memset(&info, 0, sizeof(info));
      // code using std::chrono::time_point<std::chrono::system_clock>:
      // auto timestamp = getTimestamp();
      // auto seconds = std::chrono::system_clock::to_time_t(timestamp);
      // auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
      //    timestamp - std::chrono::system_clock::from_time_t(seconds))
      //                       .count();
      // info.sec = seconds;
      // info.usec = microseconds;

      // code using doocs::Timestamp
      auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
      info.sec = sinceEpoch.seconds;
      info.usec = sinceEpoch.microseconds;
      if(_macroPulseNumberSource != nullptr) {
        info.ident = _macroPulseNumberSource->accessData(0);
      }
      else {
        info.ident = 0;
      }
      auto ret = d_fct->send(&info);
      if(ret) {
        std::cout << "ZeroMQ sending failed!!!" << std::endl;
      }
    }
  }

  void PropertyBase::updateOthers(bool handleLocking) {
    if(handleLocking) {
      getEqFct()->unlock();
    }
    for(auto& prop : otherPropertiesToUpdate) {
      if(handleLocking) {
        prop->getEqFct()->lock();
      }
      prop->updateDoocsBuffer({});
      if(handleLocking) {
        prop->getEqFct()->unlock();
      }
    }
    if(handleLocking) {
      getEqFct()->lock();
    }
  }

  void PropertyBase::setMacroPulseNumberSource(
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
    _macroPulseNumberSource = macroPulseNumberSource;
    if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
      _consistencyGroup.add(macroPulseNumberSource);
      _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct,
          std::bind(&PropertyBase::updateDoocsBuffer, this, macroPulseNumberSource->getId()));
    }
    else {
      // We don't need to match up anything with it when it changes, but we have to register this at least once
      // so the macropulse number will be included in the readAnyGroup in the updater if
      // <data_matching> is none everywhere
      _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct, []() {});
    }
  }

} // namespace ChimeraTK
