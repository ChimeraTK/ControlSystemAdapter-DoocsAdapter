#include "DoocsUpdater.h"

namespace ChimeraTK{

  void DoocsUpdater::addVariable( mtca4u::TransferElement & variable, std::function<void ()> updaterFunction){
    _elementsToRead.push_back( std::reference_wrapper< mtca4u::TransferElement > (variable) );
    _toDoocsUpdateMap[&variable]=updaterFunction;
  }

  void DoocsUpdater::update(){
    for ( auto & transferElement : _elementsToRead ){
      if (transferElement.get().readNonBlocking()){
	_toDoocsUpdateMap[&(transferElement.get())]();
      }
    }
  }

}//namespace ChimeraTK
