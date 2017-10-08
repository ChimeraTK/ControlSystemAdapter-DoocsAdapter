#include "DoocsUpdater.h"

namespace ChimeraTK{

  void DoocsUpdater::addVariable( mtca4u::TransferElement & variable, std::function<void ()> updaterFunction){
    _elementsToRead.push_back( std::reference_wrapper< mtca4u::TransferElement > (variable) );
    _toDoocsUpdateMap[&variable]=updaterFunction;
  }

  void DoocsUpdater::update(){
    for ( auto & mapElem : _toDoocsUpdateMap ){
      if (mapElem.first->readNonBlocking()){
        mapElem.second();
      }
    }
  }

}//namespace ChimeraTK
