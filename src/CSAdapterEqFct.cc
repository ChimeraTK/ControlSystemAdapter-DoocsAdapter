#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"
#include "VariableMapper.h"
#include "DoocsUpdater.h"

namespace ChimeraTK{

  bool CSAdapterEqFct::emptyLocationVariablesHandled = false;
  

  CSAdapterEqFct::CSAdapterEqFct(int fctCode,
    boost::shared_ptr<ControlSystemPVManager> const & controlSystemPVManager,
    std::string fctName)
    // The second argument in EqFct has to be a pointer to string, and NULL pointer is
    // used when the name is coming from the config file. This interface is so ugly that
    // I changed it to std::string and need the ?: trick to get a NULL pointer in 
    // if the string is empty
    : EqFct ("NAME = CSAdapterEqFct", fctName.empty()?NULL:&fctName),
     controlSystemPVManager_(controlSystemPVManager),
     fctCode_(fctCode){
    
    syncUtility_.reset (new ChimeraTK::ControlSystemSynchronizationUtility(controlSystemPVManager_));
    registerProcessVariablesInDoocs();
  }
  
  void CSAdapterEqFct::init(){
    std::cout << "this is eqfct init of " << fct_name() << std::endl;
  }

  void CSAdapterEqFct::update(){
    // Sending is done automatically when the "to device" variable is updated by Doocs.
    // No action needed here.
    // Call receive(), which triggers all receiveListeners and updates
    // the "deviceToControlSystem" variables, so Doocs knows the current values.
    // Only do this for the variables in this EqFct. All others have no callbacks in
    // this sync utility.
    syncUtility_->receive(chimeraTKReceivers_);
    // The synchronisation towards doocs is done by the updater at the moment.
    updater_.update();
  }
    
  int CSAdapterEqFct::fct_code(){
    return fctCode_;
  }
  


  void CSAdapterEqFct::registerProcessVariablesInDoocs(){
    // We only need the factory inside this function
    DoocsPVFactory factory(this, updater_, syncUtility_, controlSystemPVManager_);

    auto mappingForThisLocation = VariableMapper::getInstance().getPropertiesInLocation(fct_name());
    doocsProperties_.reserve( mappingForThisLocation.size() );

    for (auto & pvNameAndPropertyDescrition : mappingForThisLocation){
      //      doocsProperties_.push_back( factory.new_create( pvNameAndPropertyDescrition ) );
      
      auto pvName = pvNameAndPropertyDescrition.first;
      // we just need the pv name, not the description yet. The factory does that for us.
      auto chimeraTkVariable = controlSystemPVManager_->getProcessVariable(pvName);

      doocsProperties_.push_back( factory.create( chimeraTkVariable ) );
      //FIXME: Hack to keep spectra from being added to the list for the sync util. They
      //are already switched to the new updater scheme. Remove everything below if we got rid of
      //the syncutil
      if (boost::dynamic_pointer_cast<D_spectrum>(doocsProperties_.back())){
        continue;
      }
      
      // we also have to remember which chimeraTK variables we have to receive
      if ( chimeraTkVariable->isReadable() ){
	chimeraTKReceivers_.push_back(chimeraTkVariable);
      }
    }
  }

}// namespace ChimeraTK
