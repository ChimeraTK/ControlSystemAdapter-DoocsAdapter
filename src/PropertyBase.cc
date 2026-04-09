// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "PropertyBase.h"

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"

namespace ChimeraTK {

  PropertyBase::PropertyBase(
      std::string doocsPropertyName, DoocsUpdater& updater, DataConsistencyGroup::MatchingMode matchingMode)
  : _consistencyGroup(matchingMode), _doocsPropertyName(std::move(doocsPropertyName)), _doocsUpdater(updater) {}

  void PropertyBase::registerVariable(TransferElementAbstractor& var, bool update) {
    if(var.isReadable()) {
      auto id = var.getId();
      _consistencyGroup.add(var);
      if(update) {
        _doocsUpdater.addVariable(var, getEqFct(), [this, id] { return updateDoocsBuffer(id); });
      }
      else {
        _doocsUpdater.addVariable(var, getEqFct());
      }
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
    assert(_outputVarForVersionNum);
    TransferElementID compareTo = _outputVarForVersionNum->getId();

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

            std::cout << "WARNING: Data loss in property " << getEqFct()->name() << "/" << _doocsPropertyName << reason
                      << " (repeated " << _nDataLossWarnings << " times)." << std::endl;
          }
        }
        // Tracking that data is modified is only done for the "compareTo" variable.
        // If we want to detect data loss on all variables in the consistency group, we would have to track this for
        // each variable separately.
        _doocsSuccessfullyUpdated = false;
      }
      return false;
    }
    _doocsSuccessfullyUpdated = true;
    return true;
  }

  doocs::Timestamp PropertyBase::getTimestamp() {
    assert(_outputVarForVersionNum);
    doocs::Timestamp timestamp(_outputVarForVersionNum->getVersionNumber().getTime());
    return timestamp;
  }

  doocs::Timestamp PropertyBase::correctDoocsTimestamp() {
    doocs::Timestamp timestamp = getTimestamp();
    auto* d_fct = getDfct();
    // Make sure we never send out two absolute identical time stamps. If we would do so, the "watchdog" which
    // corrects inconsistencies in ZeroMQ subscriptions between sender and subscriber cannot detect the inconsistency.
    // This check also enforces that the timestamp is always increasing (never going backwards in time)
    if(timestamp <= d_fct->get_timestamp()) {
      timestamp = d_fct->get_timestamp() + std::chrono::microseconds(1);
    }

    // update global time stamp of DOOCS, but only if our time stamp is newer
    if(get_global_timestamp() < timestamp) {
      set_global_timestamp(timestamp);
    }
    d_fct->set_timestamp(timestamp);
    return timestamp;
  }

  void PropertyBase::sendZMQ(doocs::Timestamp timestamp) {
    auto* d_fct = getDfct();
    // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    if(_publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
      dmsg_info info{};
      auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
      info.sec = sinceEpoch.seconds;
      info.usec = sinceEpoch.microseconds;
      if(_macroPulseNumberSource.isInitialised()) {
        info.ident = _macroPulseNumberSource;
      }
      else {
        info.ident = 0;
      }
      dmsg_error(&info, d_fct->d_error());
      auto ret = d_fct->send(&info);
      if(ret) {
        std::cout << "ZeroMQ sending failed!!!" << std::endl;
      }
    }
  }

  void PropertyBase::updateOthers(bool handleLocking) {
    // Release the location lock before calling callbacks to avoid deadlocks when callbacks lock other locations
    if(handleLocking) {
      getEqFct()->unlock();
    }
    // Invoke all registered callbacks, passing this property as the caller so callbacks can identify the source
    for(const auto& callback : callbacksOnChange()) {
      callback(handleLocking, shared_from_this());
    }
    if(handleLocking) {
      getEqFct()->lock();
    }
  }

  void PropertyBase::setMacroPulseNumberSource(const std::string& sourcePath) {
    if(!sourcePath.empty()) {
      auto mpnSource = _doocsUpdater.getMappedProcessVariable<int64_t>(sourcePath);
      if(mpnSource->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnSource->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnSource->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnSource->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnSource->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      setMacroPulseNumberSource(mpnSource);
    }
  }

  void PropertyBase::setMacroPulseNumberSource(
      const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>& macroPulseNumberSource) {
    _macroPulseNumberSource.replace(macroPulseNumberSource);

    // we send out updates only when configured data_matching not equal 'none'
    registerVariable(
        _macroPulseNumberSource, _consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none);
  }

  void PropertyBase::setIsWriteableSource(const std::string& sourcePath) {
    if(!sourcePath.empty()) {
      auto isWriteableSource = _doocsUpdater.getMappedProcessVariable<ChimeraTK::Boolean>(sourcePath);
      if(isWriteableSource->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error(std::format(
            "The property '{}' is used as a is-writeable source, but is not a scalar.", isWriteableSource->getName()));
      }

      if(isWriteableSource->isReadable()) {
        // Readable PV (device-to-CS direction): register with DoocsUpdater to receive updates via the update loop
        _isWriteableSource.replace(isWriteableSource);
        _doocsUpdater.addVariable(_isWriteableSource, getEqFct(), [this] {
          if(bool(_isWriteableSource)) {
            this->getDfct()->set_rw_access();
          }
          else {
            this->getDfct()->set_ro_access();
          }
        });
      }
      else {
        // Write-only PV (CS-to-device direction): cannot use DoocsUpdater since that only handles readable PVs.
        // Instead, register a callback in writeableVariablesWithMultipleProperties so it gets called when another
        // property writes to the same PV (via updateOthers). The callback reads the current value from the caller's
        // DOOCS property using the generic EqData interface.
        assert(!doocsAdapter.writeableVariablesWithMultiplePropertiesIsFinal);
        auto weakSelf = weak_from_this();
        doocsAdapter.writeableVariablesWithMultipleProperties[sourcePath].emplace_back(
            [weakSelf](bool handleLocking, const boost::shared_ptr<PropertyBase>& caller) {
              auto self = weakSelf.lock();
              if(!self) {
                return;
              }
              EqData input, result;
              caller->getDfct()->get(nullptr, &input, &result, caller->getEqFct());
              bool value = result.get_bool();
              if(handleLocking) {
                self->getEqFct()->lock();
              }
              if(value) {
                self->getDfct()->set_rw_access();
              }
              else {
                self->getDfct()->set_ro_access();
              }
              if(handleLocking) {
                self->getEqFct()->unlock();
              }
            });
      }
    }
  }

  void PropertyBase::subscribeToSharedPV(const std::string& pvName) {
    // Register a callback that updates this property's DOOCS buffer when another property writes to the shared PV.
    // Uses weak_ptr to avoid preventing destruction of this property.
    auto weakSelf = weak_from_this();
    doocsAdapter.writeableVariablesWithMultipleProperties[pvName].emplace_back(
        [weakSelf](bool handleLocking, const boost::shared_ptr<PropertyBase>& caller) {
          auto self = weakSelf.lock();
          if(!self) {
            return;
          }
          assert(caller != self);

          if(handleLocking) {
            self->getEqFct()->lock();
          }
          self->updateDoocsBuffer({});
          if(handleLocking) {
            self->getEqFct()->unlock();
          }
        });
  }

  PVChangeListeners& PropertyBase::callbacksOnChange() {
    if(_callbacksCacheIsFinal) {
      return _callbacksOnChange_cache;
    }

    // Collect all callbacks from PV groups this property is subscribed to. This includes both data-property
    // callbacks (from subscribeToSharedPV) and isWriteableSource callbacks (from setIsWriteableSource).
    _callbacksOnChange_cache.clear();
    _hasOtherDataProperties = false;
    for(auto& [pvName, listeners] : doocsAdapter.writeableVariablesWithMultipleProperties) {
      if(!_sharedPVSubscriptions.contains(pvName)) {
        continue;
      }
      _callbacksOnChange_cache.insert(_callbacksOnChange_cache.end(), listeners.begin(), listeners.end());
      // More than one listener means there are other data properties besides just an isWriteableSource callback
      if(listeners.size() > 1) {
        _hasOtherDataProperties = true;
      }
    }
    if(doocsAdapter.writeableVariablesWithMultiplePropertiesIsFinal) {
      _callbacksCacheIsFinal = true;
    }
    return _callbacksOnChange_cache;
  }

} // namespace ChimeraTK
