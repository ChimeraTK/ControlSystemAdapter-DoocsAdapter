#include "DoocsUpdater.h"

#include <ChimeraTK/ReadAnyGroup.h>
#include <unordered_set>

namespace ChimeraTK {

  void DoocsUpdater::addVariable(const TransferElementAbstractor& variable,
      EqFct* eq_fct,
      std::function<void()>
          updaterFunction) {
    // Don't add the transfer element twice into the list of elements to read.
    // To check if there is such an element we use the map with the lookup table
    // which has a search function, instead of manually looking at the elements in
    // the list and compare the ID.
    if(_toDoocsUpdateMap.find(variable.getId()) == _toDoocsUpdateMap.end()) {
      _elementsToRead.push_back(variable);
    }

    _toDoocsUpdateMap[variable.getId()].push_back(updaterFunction);
    _toDoocsEqFctMap[variable.getId()].push_back(eq_fct);
  }

  void DoocsUpdater::update() {
    for(auto& transferElem : _elementsToRead) {
      if(transferElem.readLatest()) {
        for(auto& updaterFunction : _toDoocsUpdateMap[transferElem.getId()]) {
          updaterFunction();
        }
      }
    }
  }

  void DoocsUpdater::updateLoop() {
    if(_elementsToRead.empty()) {
      return;
    }

    // Worst-case: We need to lock all locations, so pre-allocate this here
    std::unordered_set<EqFct*> locationsToLock;
    locationsToLock.reserve(_toDoocsEqFctMap.size());

    ReadAnyGroup group(_elementsToRead.begin(), _elementsToRead.end());
    while(true) {
      // Wait until any variable got an update
      auto notification = group.waitAny();
      auto updatedElement = notification.getId();
      // Gather all involved locations in a unique set
      for(auto& location : _toDoocsEqFctMap[updatedElement]) {
        if(locationsToLock.insert(location).second) location->lock();
      }
      // Complete the read transfer of the process variable
      notification.accept();
      // Call all updater functions
      for(auto& updaterFunction : _toDoocsUpdateMap[updatedElement]) updaterFunction();
      // Unlock all involved locations
      for(auto& location : locationsToLock) location->unlock();
      locationsToLock.clear();
      // Allow shutting down this thread...
      boost::this_thread::interruption_point();
    }
  }

  void DoocsUpdater::run() { _syncThread = boost::thread(boost::bind(&DoocsUpdater::updateLoop, this)); }

  void DoocsUpdater::stop() {
    _syncThread.interrupt();
    for(auto& var : _elementsToRead) {
      var.getHighLevelImplElement()->interrupt();
    }
    _syncThread.join();
  }

  DoocsUpdater::~DoocsUpdater() { stop(); }

} // namespace ChimeraTK
