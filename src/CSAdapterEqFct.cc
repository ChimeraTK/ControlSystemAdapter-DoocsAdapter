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
    : EqFct ("NAME = CSAdapterEqFct"),
     controlSystemPVManager_(controlSystemPVManager),
      fctCode_(fctCode), updater_(updater){

    std::cout << "CSAdapterEqFct::CSAdapterEqFct name is " << name() << std::endl;
    registerProcessVariablesInDoocs(fctName);
  }

  CSAdapterEqFct::~CSAdapterEqFct(){
    //stop the updater thread before any of the process variables go out of scope
    updater_->stop();
  }

  void CSAdapterEqFct::init(){
    std::cout << "this is eqfct init of " << name() << std::endl;
  }

  int CSAdapterEqFct::fct_code(){
    return fctCode_;
  }

  void CSAdapterEqFct::registerProcessVariablesInDoocs(std::string const & fctName){
    // We only need the factory inside this function
    DoocsPVFactory factory(this, *updater_, controlSystemPVManager_);

    auto mappingForThisLocation = VariableMapper::getInstance().getPropertiesInLocation(fctName);
    doocsProperties_.reserve( mappingForThisLocation.size() );

    for (auto & propertyDescrition : mappingForThisLocation){
      try {
        doocsProperties_.push_back( factory.create( propertyDescrition ) );
      }
      catch(std::invalid_argument &e) {
        std::cerr << "**** WARNING: Could not create property for variable '" << propertyDescrition->location << "/" <<
                     propertyDescrition->name << "': " << e.what() << ". Skipping this property." << std::endl;
      }
    }
  }

}// namespace ChimeraTK
