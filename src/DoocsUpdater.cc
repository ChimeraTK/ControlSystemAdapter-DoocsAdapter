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

  void DoocsUpdater::updateLoop(){
    while(true){
      update();
      // FIXME: This is brainstorming. Use testable sleep here
      boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
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
