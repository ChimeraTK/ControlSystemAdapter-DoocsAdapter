#include "DoocsUpdater.h"

namespace ChimeraTK{

  void DoocsUpdater::addVariable( mtca4u::TransferElement & variable, std::function<void ()> updaterFunction){
    _elementsToRead.push_back( std::reference_wrapper< mtca4u::TransferElement > (variable) );
    _toDoocsUpdateMap[&variable].push_back(updaterFunction);
  }

  void DoocsUpdater::update(){
    for ( auto & mapElem : _toDoocsUpdateMap ){
      ///@todo FIXME: This should be readNonBlocking(), or better readAny on the whole map
      // Currently this is consistent behaviour in the location update, and needed
      // for consistent testing.
      if (mapElem.first->readLatest()){
        for (auto & updaterFunc : mapElem.second){
          updaterFunc();
        }
      }
    }
  }

}//namespace ChimeraTK
