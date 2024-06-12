// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "StatusHandler.h"
// include above first to avoid name clash with #define from DOOCS

#include "CSAdapterEqFct.h"
#include "DoocsProcessArray.h"
#include "DoocsPVFactory.h"
#include "DoocsUpdater.h"
#include "PropertyDescription.h"
#include "VariableMapper.h"

namespace ChimeraTK {

  bool CSAdapterEqFct::emptyLocationVariablesHandled = false;

  CSAdapterEqFct::CSAdapterEqFct(int code, const EqFctParameters& p)
  : EqFct(p), controlSystemPVManager_(doocsAdapter.getControlSystemPVManager()), updater_(doocsAdapter.updater),
    _code(code) {
    registerProcessVariablesInDoocs();

    // construct and populate the StatusHandler for this location
    for(const ErrorReportingInfo& errorReportingInfo :
        ChimeraTK::VariableMapper::getInstance().getErrorReportingInfos()) {
      if(name_.get_value() != errorReportingInfo.targetLocation) continue;

      assert(!statusHandler_); // only single StatusHandler may be requested per location
      auto statusCodeVariable = controlSystemPVManager_->getProcessArray<int32_t>(errorReportingInfo.statusCodeSource);
      if(!statusCodeVariable) {
        throw ChimeraTK::logic_error("illegal/non-existing statusCodeSource: " + errorReportingInfo.statusCodeSource);
      }
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

        auto res = pair.second->set_mode(DMSG_EN, 10);
        if(res != 0) {
          throw ChimeraTK::logic_error("Could not enable ZeroMQ messaging for variable '" + pair.first->location + "/" +
              pair.first->name + "'. Code: " + std::to_string(res));
        }
      }
    }
  }

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
            saveArray<int64_t>(p);
            break;
          case DATA_A_ULONG:
            saveArray<uint64_t>(p);
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
