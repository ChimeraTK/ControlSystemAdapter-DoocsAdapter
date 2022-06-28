#include "StatusHandler.h" // include this first to avoid name clash with #define from DOOCS
#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"
#include "DoocsUpdater.h"
#include "VariableMapper.h"
#include "DoocsProcessArray.h"
#include "PropertyDescription.h"

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

    // construct and populate the StatusHandler for this location
    for(const ErrorReportingInfo& errorReportingInfo :
        ChimeraTK::VariableMapper::getInstance().getErrorReportingInfos()) {
      if(name_.get_value() != errorReportingInfo.targetLocation) continue;

      assert(!statusHandler_); // only single StatusHandler may be requested per location
      auto statusCodeVariable = controlSystemPVManager_->getProcessArray<int32_t>(errorReportingInfo.statusCodeSource);
      if(!statusCodeVariable)
        throw ChimeraTK::logic_error("illegal/non-existing statusCodeSource: " + errorReportingInfo.statusCodeSource);
      // this one is optional
      ProcessArray<std::string>::SharedPtr statusStringVariable;
      if(controlSystemPVManager_->hasProcessVariable(errorReportingInfo.statusStringSource)) {
        statusStringVariable =
            controlSystemPVManager_->getProcessArray<std::string>(errorReportingInfo.statusStringSource);
      }
      statusHandler_.reset(new StatusHandler(this, updater_, statusCodeVariable, statusStringVariable));
    }
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
          throw ChimeraTK::logic_error("Could not enable ZeroMQ messaging for variable '" + pair.first->location + "/" +
              pair.first->name + "'. Code: " + std::to_string(res));
        }
      }
    }
  }

  int CSAdapterEqFct::fct_code() { return fctCode_; }

  int CSAdapterEqFct::write(std::ostream& fprt) {
    /*
     * iterate over all properties (i.e. instances of D_fcn),
     * check if its a doocs::D_array()
     * if it should be persisted and it's not by default due to the restricting length MAX_CONF_LENGTH,
     * persist it in a separate file, but only if writable.
     */
    for(auto& pair : this->doocsProperties_) {
      // try a side-cast to get property attributes
      auto attrs = std::dynamic_pointer_cast<PropertyAttributes>(pair.first);
      if(attrs && attrs->persist == PersistConfig::ON) {
        D_fct* p = pair.second.get();

        switch(p->data_type()) {
          case DATA_A_BYTE:
            saveArray<unsigned char>(p);
            break;
          case DATA_A_SHORT:
            saveArray<short>(p);
            break;
          case DATA_A_USHORT:
            saveArray<unsigned short>(p);
            break;
          case DATA_A_INT:
            saveArray<int>(p);
            break;
          case DATA_A_UINT:
            saveArray<unsigned int>(p);
            break;
          case DATA_A_LONG:
            saveArray<long long>(p);
            break;
          case DATA_A_ULONG:
            saveArray<unsigned long long>(p);
            break;
          case DATA_A_FLOAT:
            saveArray<float>(p);
            break;
          case DATA_A_DOUBLE:
            saveArray<double>(p);
            break;
          // case DATA_A_TS_SPECTRUM does not need handling here, it's already persisted by base implementation.
        }
      }
    }

    return EqFct::write(fprt);
  }

  template<class ValueType>
  void CSAdapterEqFct::saveArray(D_fct* p) {
    auto* arr = dynamic_cast<DoocsProcessArray<doocs::D_array<ValueType>, ValueType>*>(p);
    if(arr && arr->length() > MAX_CONF_LENGTH && arr->modified) {
      arr->modified = false;
      arr->save();
    }
  }

  void CSAdapterEqFct::registerProcessVariablesInDoocs() {
    // We only need the factory inside this function
    DoocsPVFactory factory(this, *updater_, controlSystemPVManager_);

    auto mappingForThisLocation = VariableMapper::getInstance().getPropertiesInLocation(name());

    for(auto& propertyDescription : mappingForThisLocation) {
      try {
        doocsProperties_[propertyDescription] = factory.create(propertyDescription);

        // if one of the PVs used by the property is among the keys of the writeableVariablesWithMultipleProperties map,
        // add the property to the list of properties to update (value of the beforementioned map).
        auto& theMap = doocsAdapter.writeableVariablesWithMultipleProperties;
        for(const auto& pvNameUsedByProperty : propertyDescription->getSources()) {
          if(theMap.find(pvNameUsedByProperty) != theMap.end()) {
            // the PV name has been found in the map keys -> add the property to the list to update
            auto p = boost::dynamic_pointer_cast<ChimeraTK::PropertyBase>(doocsProperties_.at(propertyDescription));
            auto& listOfPropertiesToUpdate = theMap.at(pvNameUsedByProperty);
            listOfPropertiesToUpdate.insert(p);
          }
        }
      }
      catch(std::invalid_argument& e) {
        std::cerr << "**** WARNING: Could not create property for variable '" << propertyDescription->location << "/"
                  << propertyDescription->name << "': " << e.what() << ". Skipping this property." << std::endl;
      }
      catch(doocs::Error& e) {
        std::cerr << "**** WARNING: Could not create property for variable '" << propertyDescription->location << "/"
                  << propertyDescription->name << "': " << e.what() << ". Skipping this property." << std::endl;
      }
    }
  }

} // namespace ChimeraTK
