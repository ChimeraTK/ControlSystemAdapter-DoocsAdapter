// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsUpdater.h"

#include <ChimeraTK/ReadAnyGroup.h>

#include <unordered_set>

namespace ChimeraTK {

  void DoocsUpdater::addVariable(
      TransferElementAbstractor variable, EqFct* eq_fct, const std::function<void()>& updaterFunction) {
    // Don't add the transfer element twice into the list of elements to read (i.e. later into the ReadAnyGroup).
    // To check if there is such an element we use the map with the lookup table
    // which has a search function, instead of manually looking at the elements in
    // the list and compare the ID.
    if(_toDoocsDescriptorMap.find(variable.getId()) == _toDoocsDescriptorMap.end()) {
      _elementsToRead.push_back(variable);
    }
    else {
      _toDoocsDescriptorMap[variable.getId()].additionalTransferElements.insert(variable.getHighLevelImplElement());
    }

    _toDoocsDescriptorMap[variable.getId()].updateFunctions.push_back(updaterFunction);
    _toDoocsDescriptorMap[variable.getId()].locations.push_back(eq_fct);
  }

  void DoocsUpdater::update() {
    for(auto& transferElem : _elementsToRead) {
      if(transferElem.readLatest()) {
        for(auto& updaterFunction : _toDoocsDescriptorMap[transferElem.getId()].updateFunctions) {
          updaterFunction();
        }
      }
    }
  }

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

    // Call preRead for all TEs on additional transfer elements. waitAny() is doing this for all elements in the
    // ReadAnyGroup. Unnecessary calls to preRead() are anyway ignored and merely pose a performance issue. For large
    // servers, the performance impact is significant, hence we keep track of the TEs which need to be called.
    for(auto& pair : _toDoocsDescriptorMap) {
      for(const auto& elem : pair.second.additionalTransferElements) {
        elem->preRead(ChimeraTK::TransferType::read);
      }
    }

    while(true) {
      // Wait until any variable got an update
      auto notification = group.waitAny();
      auto updatedElement = notification.getId();
      auto& descriptor = _toDoocsDescriptorMap[updatedElement];

      // Gather all involved locations in a unique set
      for(auto& location : descriptor.locations) {
        if(locationsToLock.insert(location).second) {
          location->lock();
        }
      }
      // Complete the read transfer of the process variable
      notification.accept();

      // Call postRead for all TEs on _toDoocsAdditionalTransferElementsMap for the updated ID
      for(const auto& elem : descriptor.additionalTransferElements) {
        elem->postRead(ChimeraTK::TransferType::read, true);
      }

      // Call all updater functions
      for(auto& updaterFunction : descriptor.updateFunctions) {
        updaterFunction();
      }

      // Unlock all involved locations
      for(const auto& location : locationsToLock) {
        location->unlock();
      }
      locationsToLock.clear();

      // Call preRead for all TEs on _toDoocsAdditionalTransferElementsMap for the updated ID
      for(const auto& elem : descriptor.additionalTransferElements) {
        elem->preRead(ChimeraTK::TransferType::read);
      }

      // Allow shutting down this thread...
      boost::this_thread::interruption_point();
    }
  }

  void DoocsUpdater::run() {
    _syncThread = boost::thread([this] { updateLoop(); });
  }

  void DoocsUpdater::stop() {
    _syncThread.interrupt();
    for(auto& var : _elementsToRead) {
      var.getHighLevelImplElement()->interrupt();
    }
    _syncThread.join();
  }

  DoocsUpdater::~DoocsUpdater() {
    stop();
  }

} // namespace ChimeraTK
