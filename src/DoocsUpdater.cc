// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsUpdater.h"

#include <ChimeraTK/cppext/threadName.hpp>
#include <ChimeraTK/ReadAnyGroup.h>

#include <unordered_set>

namespace ChimeraTK {

  void DoocsUpdater::addVariable(
      const TransferElementAbstractor& variable, EqFct* eq_fct, const std::function<void()>& updaterFunction) {
    // Don't add the transfer element twice into the list of elements to read (not allowed with ReadAnyGroup).
    if(_toDoocsDescriptorMap.find(variable.getId()) == _toDoocsDescriptorMap.end()) {
      _elementsToRead.push_back(variable);
    }

    if(updaterFunction) {
      _toDoocsDescriptorMap[variable.getId()].updateFunctions.push_back(updaterFunction);
    }
    if(eq_fct) {
      _toDoocsDescriptorMap[variable.getId()].locations.push_back(eq_fct);
    }
  }

  /********************************************************************************************************************/

  void DoocsUpdater::update() {
    for(auto& transferElem : _elementsToRead) {
      if(transferElem.readLatest()) {
        for(auto& updaterFunction : _toDoocsDescriptorMap[transferElem.getId()].updateFunctions) {
          updaterFunction();
        }
      }
    }
  }

  /********************************************************************************************************************/

  void DoocsUpdater::updateLoop() {
    if(_elementsToRead.empty()) {
      return;
    }

    std::unordered_set<EqFct*> locationsToLock;

    // preallocate locationsToLock to avoid breaking "realtime" behaivour
    size_t nMaxLocationsToLock = 0;
    for(auto& pair : _toDoocsDescriptorMap) {
      nMaxLocationsToLock = std::max(nMaxLocationsToLock, pair.second.locations.size());
    }
    locationsToLock.reserve(nMaxLocationsToLock);

    ReadAnyGroup group(_elementsToRead.begin(), _elementsToRead.end());

    while(true) {
      // Wait until any variable got an update
      auto notification = group.waitAny();
      auto updatedElement = notification.getId();
      auto& descriptor = _toDoocsDescriptorMap[updatedElement];

      bool isFanSource = routing.isFanSource(updatedElement);
      if(!isFanSource) {
        // Gather all involved locations in a unique set
        for(auto& location : descriptor.locations) {
          if(locationsToLock.insert(location).second) {
            location->lock();
          }
        }
      }
      // Complete the read transfer of the process variable.
      if(notification.accept()) {
        auto te = notification.getTransferElement();
        assert(notification.getTransferElement().getVersionNumber() > VersionNumber{nullptr});
        if(isFanSource) {
          // if updated process var is source for a fan-out, generate the copies
          // We assume that all elements (source and copies) are in our ReadAnyGroup
          // Do not send out updates via DOOCS if the update is for source of a fan-out.
          assert(descriptor.updateFunctions.empty());
          routing.send(updatedElement);
        }
        else {
          // Call all updater functions
          for(auto& updaterFunction : descriptor.updateFunctions) {
            updaterFunction();
          }
        }
      }
      // Unlock all involved locations
      for(const auto& location : locationsToLock) {
        location->unlock();
      }
      locationsToLock.clear();

      // Allow shutting down this thread...
      boost::this_thread::interruption_point();
    }
  }

  /********************************************************************************************************************/

  void DoocsUpdater::run() {
    _syncThread = boost::thread([this] {
      cppext::setThreadName("DoocsUpdater");
      updateLoop();
    });
  }

  /********************************************************************************************************************/

  void DoocsUpdater::stop() {
    if(_syncThread.joinable()) {
      _syncThread.interrupt();
      for(auto& var : _elementsToRead) {
        var.getHighLevelImplElement()->interrupt();
      }
      _syncThread.join();
    }
  }

  /********************************************************************************************************************/

  DoocsUpdater::~DoocsUpdater() {
    stop();
  }

  /********************************************************************************************************************/

  TransferElement::SharedPtr DoocsUpdater::getMappedProcessVariableUnTyped(
      const ChimeraTK::RegisterPath& processVariableName) {
    auto pv = _controlSystemPVManager->getProcessVariable(processVariableName);

    // Note about bi-directional PVs: they are not covered here, because the RoutingDecorator cannot handle them. This
    // may be an issue if bi-directional PVs are mapped more than once to DOOCS (e.g. one writeable and one or more
    // read-only copy) with different types, but this does also not work for write-only properties yet.
    if(pv->isWriteable()) {
      return pv;
    }
    bool sourceRequiresFan = _pvNamesWithFan.contains(pv->getName());
    if(!sourceRequiresFan) {
      return pv;
    }

    // We add the source for the fan to elementsToRead.
    // We don't need a doocsUpdater function for the source.
    TransferElementID sourceId = pv->getId();
    if(!_toDoocsDescriptorMap.contains(sourceId)) {
      _elementsToRead.emplace_back(pv);
      _toDoocsDescriptorMap[sourceId];
    }

    return routing.add(pv);
  }

} // namespace ChimeraTK
