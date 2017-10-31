#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"
#include "VariableMapper.h"
#include "DoocsUpdater.h"

namespace ChimeraTK{

  bool CSAdapterEqFct::emptyLocationVariablesHandled = false;
  

  CSAdapterEqFct::CSAdapterEqFct(int fctCode,
    boost::shared_ptr<ControlSystemPVManager> const & controlSystemPVManager,
    boost::shared_ptr<DoocsUpdater> const & updater, std::string fctName )
    // The second argument in EqFct has to be a pointer to string, and NULL pointer is
    // used when the name is coming from the config file. This interface is so ugly that
    // I changed it to std::string and need the ?: trick to get a NULL pointer in 
    // if the string is empty
    : EqFct ("NAME = CSAdapterEqFct", fctName.empty()?NULL:&fctName),
     controlSystemPVManager_(controlSystemPVManager),
      fctCode_(fctCode), updater_(updater){
    
    registerProcessVariablesInDoocs();
  }
  
  CSAdapterEqFct::~CSAdapterEqFct(){
    //stop the updater thread before any of the process variables go out of scope
    updater_->stop();
  }

  void CSAdapterEqFct::init(){
    std::cout << "this is eqfct init of " << fct_name() << std::endl;
  }

  int CSAdapterEqFct::fct_code(){
    return fctCode_;
  }

  void CSAdapterEqFct::registerProcessVariablesInDoocs(){
    // We only need the factory inside this function
    DoocsPVFactory factory(this, *updater_, controlSystemPVManager_);

    auto mappingForThisLocation = VariableMapper::getInstance().getPropertiesInLocation(fct_name());
    doocsProperties_.reserve( mappingForThisLocation.size() );

    for (auto & propertyDescrition : mappingForThisLocation){
      doocsProperties_.push_back( factory.create( propertyDescrition ) );
    }
  }

}// namespace ChimeraTK
