#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"
#include "DoocsUpdater.h"
#include "VariableMapper.h"

namespace ChimeraTK {

  bool CSAdapterEqFct::emptyLocationVariablesHandled = false;

  CSAdapterEqFct::CSAdapterEqFct(int fctCode, boost::shared_ptr<ControlSystemPVManager> const& controlSystemPVManager,
      boost::shared_ptr<DoocsUpdater> const& updater, std::string fctName)
  // The second argument in EqFct has to be a pointer to string, and NULL
  // pointer is used when the name is coming from the config file. This
  // interface is so ugly that I changed it to std::string and need the ?:
  // trick to get a NULL pointer in if the string is empty
  : EqFct("NAME = CSAdapterEqFct"), controlSystemPVManager_(controlSystemPVManager), fctCode_(fctCode),
    updater_(updater) {
    // When testing the EqFct stand alone, the name is not set properly. Do this
    // with the additional parameter of this constructor.
    if(name().empty()) {
      name_.assign(fctName);
    }
    registerProcessVariablesInDoocs();
  }

  CSAdapterEqFct::~CSAdapterEqFct() {
    // stop the updater thread before any of the process variables go out of scope
    updater_->stop();
  }

  void CSAdapterEqFct::init() {}

  void CSAdapterEqFct::post_init() {
    for(auto& pair : doocsProperties_) {
      auto attrs = std::dynamic_pointer_cast<PropertyAttributes>(pair.first);
      assert(attrs != nullptr);
      if(attrs->publishZMQ) {
        auto res = pair.second->set_mode(DMSG_EN);
        if(res != 0) {
          throw ChimeraTK::logic_error(
              "Could not enable ZeroMQ messaging for prop_someZMQInt. Code: " + std::to_string(res));
        }
      }
    }
  }

  int CSAdapterEqFct::fct_code() { return fctCode_; }

  void CSAdapterEqFct::registerProcessVariablesInDoocs() {
    // We only need the factory inside this function
    DoocsPVFactory factory(this, *updater_, controlSystemPVManager_);

    auto mappingForThisLocation = VariableMapper::getInstance().getPropertiesInLocation(name());

    for(auto& propertyDescription : mappingForThisLocation) {
      try {
        doocsProperties_[propertyDescription] = factory.create(propertyDescription);
      }
      catch(std::invalid_argument& e) {
        std::cerr << "**** WARNING: Could not create property for variable '" << propertyDescription->location << "/"
                  << propertyDescription->name << "': " << e.what() << ". Skipping this property." << std::endl;
      }
    }
  }

} // namespace ChimeraTK
