// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "PropertyBase.h"

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"

namespace ChimeraTK {

  void PropertyBase::registerVariable(TransferElementAbstractor& var) {
    if(var.isReadable()) {
      auto id = var.getId();
      _consistencyGroup.add(var);
      _doocsUpdater.addVariable(var, getEqFct(), [this, id] { return updateDoocsBuffer(id); });
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
      if(_macroPulseNumberSource != nullptr) {
        info.ident = _macroPulseNumberSource->accessData(0);
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
    if(handleLocking) {
      getEqFct()->unlock();
    }
    for(const auto& weakProp : otherPropertiesToUpdate) {
      auto prop = weakProp.lock();
      if(!prop) {
        // property went away, could happen in shutdown phase
        continue;
      }
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
      const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>& macroPulseNumberSource) {
    _macroPulseNumberSource = macroPulseNumberSource;
    if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
      registerVariable(_macroPulseNumberSource);
    }
    else {
      // We don't need to match up anything with it when it changes, but we have to register this at least once
      // so the macropulse number will be included in the readAnyGroup in the updater if
      // <data_matching> is none everywhere
      _doocsUpdater.addVariable(
          ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), getEqFct(), []() {});
    }
  }

} // namespace ChimeraTK
