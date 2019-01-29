#include "DoocsUpdater.h"

#include <unordered_set>
#include <ChimeraTK/ReadAnyGroup.h>

namespace ChimeraTK{

  void DoocsUpdater::addVariable( const TransferElementAbstractor &variable, EqFct *eq_fct, std::function<void ()> updaterFunction){
    // Don't add the transfer element twice into the list of elements to read.
    // To check if there is such an element we use the map with the lookup table
    // which has a search function, instead of manually looking at the elements in the list
    // and compare the ID.
    if ( _toDoocsUpdateMap.find(variable.getId()) == _toDoocsUpdateMap.end() ){
      _elementsToRead.push_back(variable);
    }

    _toDoocsUpdateMap[variable.getId()].push_back(updaterFunction);
    _toDoocsEqFctMap[variable.getId()].push_back(eq_fct);
  }

  void DoocsUpdater::update(){
    for ( auto & transferElem : _elementsToRead ){
      if (transferElem.readLatest()){
        for (auto & updaterFunction : _toDoocsUpdateMap[transferElem.getId()]){
          updaterFunction();
        }
      }
    }
  }

  void DoocsUpdater::updateLoop(){
    if (_elementsToRead.empty()) {
        return;
    }

    ReadAnyGroup group(_elementsToRead.begin(), _elementsToRead.end());
    while(true) {
      // Wait until any variable got an update
      auto updatedElement = group.waitAny();
      // Gather all involved locations in a unique set
      std::unordered_set<EqFct*> locationsToLock;
      for(auto &location : _toDoocsEqFctMap[updatedElement]) locationsToLock.insert(location);
      // Lock all involved locations
      for(auto &location : locationsToLock) location->lock();
      // Complete the read transfer of the process variable
      group.postRead();
      // Call all updater functions
      for(auto &updaterFunction : _toDoocsUpdateMap[updatedElement]) updaterFunction();
      // Unlock all involved locations
      for(auto &location : locationsToLock) location->unlock();
      // Allow shutting down this thread...
      boost::this_thread::interruption_point();
    }
  }

  void DoocsUpdater::run(){
    _syncThread = boost::thread( boost::bind( &DoocsUpdater::updateLoop, this) );
  }

  void DoocsUpdater::stop(){
    _syncThread.interrupt();
    _syncThread.join();
  }

  DoocsUpdater::~DoocsUpdater(){
    stop();
  }


}//namespace ChimeraTK
