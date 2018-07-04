#include "DoocsUpdater.h"

#include <ChimeraTK/ReadAnyGroup.h>

namespace ChimeraTK{

  void DoocsUpdater::addVariable( const TransferElementAbstractor &variable, std::function<void ()> updaterFunction){
    // Don't add the transfer element twice into the list of elements to read.
    // To check if there is such an element we use the map with the lookup table
    // which has a search function, instead of manually looking at the elements in the list
    // and compare the ID.
    if ( _toDoocsUpdateMap.find(variable.getId()) == _toDoocsUpdateMap.end() ){
      _elementsToRead.push_back(variable);
    }

    _toDoocsUpdateMap[variable.getId()].push_back(updaterFunction);
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
    ReadAnyGroup group(_elementsToRead.begin(), _elementsToRead.end());
    while(true){
      auto updatedElement = group.readAny();
      for (auto & updaterFunction : _toDoocsUpdateMap[updatedElement]){
        updaterFunction();
      }
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
