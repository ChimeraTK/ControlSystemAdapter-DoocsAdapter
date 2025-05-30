// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsAdapter.h"

#include "CSAdapterEqFct.h"
#include "DoocsUpdater.h"
#include "getAllVariableNames.h"
#include "PropertyDescription.h"
#include "VariableMapper.h"

#include <filesystem>

namespace ChimeraTK {

  /********************************************************************************************************************/

  std::atomic<bool> DoocsAdapter::isInitialised(false);

  static char const* XML_CONFIG_SUFFIX = "-DoocsVariableConfig.xml";

  /********************************************************************************************************************/

  DoocsAdapter::DoocsAdapter() {
    // Create the managers. We need both
    std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
        createPVManager();

    _controlSystemPVManager = pvManagers.first;
    _devicePVManager = pvManagers.second;

    updater = boost::make_shared<DoocsUpdater>();
  }

  /********************************************************************************************************************/

  boost::shared_ptr<DevicePVManager> const& DoocsAdapter::getDevicePVManager() const {
    return _devicePVManager;
  }

  /********************************************************************************************************************/

  boost::shared_ptr<ControlSystemPVManager> const& DoocsAdapter::getControlSystemPVManager() const {
    return _controlSystemPVManager;
  }

  /********************************************************************************************************************/

  void DoocsAdapter::waitUntilInitialised() {
    int i = 0;
    while(true) {
      if(isInitialised) {
        return;
      }
      // just sleep a bit. Use the "cheap" usleep, we don't care about precision
      // here
      ++i;
      usleep(100);
    }
  }

  /********************************************************************************************************************/

  bool DoocsAdapter::checkPrintDataLossWarning(size_t counter) {
    // print first time at counter == 10 to suppress the messages spamming at startup
    if(counter < 1) {
      return false;
    }
    if(counter < 100) {
      return counter % 10 == 0;
    }
    if(counter < 1000) {
      return counter % 100 == 0;
    }
    if(counter < 10000) {
      return counter % 1000 == 0;
    }
    if(counter < 100000) {
      return counter % 10000 == 0;
    }
    if(counter < 1000000) {
      return counter % 100000 == 0;
    }
    return counter % 1000000 == 0; // if the rate is 10Hz, this is roughly once per day
  }

  /********************************************************************************************************************/

  void DoocsAdapter::beforeAutoInit() {
    // prevent concurrent execution. It is unclear whether DOOCS may call auto_init in parallel in some situations, so
    // better implement a lock.
    static std::mutex mx;
    std::unique_lock<std::mutex> lk(mx);

    // execute actions only once
    if(_before_auto_init_called) {
      return;
    }
    _before_auto_init_called = true;

    // connect properties which use the same writable PV to keep the property values consistent
    for(auto& group : doocsAdapter.writeableVariablesWithMultipleProperties) {
      for(const auto& weakProperty : group.second) {
        auto property = weakProperty.lock(); // cannot fail, DoocsAdapter has ownership via PV manager
        property->otherPropertiesToUpdate = group.second;
        property->otherPropertiesToUpdate.erase(property); // the property should not be in its own list
      }
    }
    doocsAdapter.writeableVariablesWithMultipleProperties.clear(); // save memory (information no longer needed)
  }

  /********************************************************************************************************************/

  std::unique_ptr<doocs::Server> DoocsAdapter::createServer() {
    auto server = std::make_unique<doocs::Server>(ChimeraTK::ApplicationBase::getInstance().getName().c_str());

    server->set_init_prolog([&] { ChimeraTK::DoocsAdapter::eqInitProlog(); });
    server->set_post_init_epilog([&] { ChimeraTK::DoocsAdapter::postInitEpilog(); });
    server->set_cancel_epilog([&] { ChimeraTK::DoocsAdapter::eqCancel(); });

    // This is a work-around for not being able to create the same location type for any arbitrary location code
    // found in the conf file. It works as long as the largest code is below 10000. The registration here takes
    // around 1ms on a standard MicroTCA AMC CPU.
    // See: https://mcs-gitlab.desy.de/doocs/doocs-core-libraries/serverlib/-/issues/79
    for(int c = 2; c < 10000; ++c) {
      server->register_location_class(
          c, [=](const EqFctParameters& p) { return std::make_unique<CSAdapterEqFct>(c, p); });
    }
    // The DCM server uses the code 5522228 = 0x544334 = 'TC4', so we register it here as well. Need to remove this
    // temporary hack ASAP!
    server->register_location_class(
        5522228, [=](const EqFctParameters& p) { return std::make_unique<CSAdapterEqFct>(5522228, p); });

    // Note: We cannot go back to use eq_create(), since we have link order issues and sometimes the wrong mplementation
    // might win (the default implementation from the serverlib, which terminates the server with an error message).

    return server;
  }

  /********************************************************************************************************************/

  /* eq_init_prolog is called before the locations are created, i.e. before the
   * first call to eq_create. We initialise the application, i.e. all process
   * variables are created in this function. */
  void DoocsAdapter::eqInitProlog() {
    // set the DOOCS server name to the application name
    // Create static instances for all applications cores. They must not have
    // overlapping process variable names ("location/protery" must be unique).
    ChimeraTK::ApplicationBase::getInstance().setPVManager(doocsAdapter.getDevicePVManager());
    ChimeraTK::ApplicationBase::getInstance().initialise();

    // the variable manager can only be filled after we have the CS manager
    auto pvNames = ChimeraTK::getAllVariableNames(doocsAdapter.getControlSystemPVManager());

    auto xmlFileName = ChimeraTK::ApplicationBase::getInstance().getName() + XML_CONFIG_SUFFIX;

    if(std::filesystem::exists(xmlFileName)) {
      ChimeraTK::VariableMapper::getInstance().prepareOutput(xmlFileName, pvNames);
    }
    else {
      std::cerr << "WARNING: No XML file for the Doocs variable config found. Trying direct import." << std::endl;
      ChimeraTK::VariableMapper::getInstance().directImport(pvNames);
    }

    // prepare list of unmapped read variables and pass it to the Application for optimisation
    for(const auto& p : ChimeraTK::VariableMapper::getInstance().getUsedVariables()) {
      auto it = pvNames.find(p);
      if(it != pvNames.end()) {
        pvNames.erase(it);
      }
    }
    ChimeraTK::ApplicationBase::getInstance().optimiseUnmappedVariables(pvNames);

    // prepare list of properties connected to the same writable PV
    {
      // create map of PV name to properties using the PV (only for writable PVs)
      std::map<std::string, std::set<std::shared_ptr<ChimeraTK::PropertyDescription>>> reverseMapping;
      for(const auto& descr : ChimeraTK::VariableMapper::getInstance().getAllProperties()) {
        for(const auto& source : descr->getSources()) {
          if(!doocsAdapter.getControlSystemPVManager()->getProcessVariable(source)->isWriteable()) {
            // to not add PVs to the mapping which are not writeable
            continue;
          }
          reverseMapping[source].insert(descr);
        }
      }
      // filter the map to contain only PVs being used at least twice with at least one writable property
      for(const auto& p : reverseMapping) {
        if(p.second.size() < 2) {
          continue;
        }
        size_t writeableCount = 0;
        for(const auto& d : p.second) {
          auto attr = std::dynamic_pointer_cast<ChimeraTK::PropertyAttributes>(d);
          if(attr->isWriteable) {
            ++writeableCount;
          }
        }
        if(writeableCount == 0) {
          continue;
        }
        if(writeableCount > 1) {
          // This case is not (yet) covered. It would require some synchronisation mechanism between writeable
          // properties, which is not trivial as inconsistencies and dead locks must be avoided.
          std::cout << "**** WARNING: Variable '" + p.first +
                  "' mapped to more than one writeable property. Expect inconsistencies!\n";
        }

        // Add PVs which are used at least twice with at least one writable property to list
        doocsAdapter.writeableVariablesWithMultipleProperties[p.first] = {};
      }
    }
  }

  /********************************************************************************************************************/

  /* post_init_epilog is called after all DOOCS properties are fully intialised,
   * including any value intialisation from the config file. We start the
   * application here. It will be launched in a separate thread. */
  void DoocsAdapter::postInitEpilog() {
    // check for locations not yet created (due to missing entries in the .conf file) and create them now
    std::map<std::string, int> locMap = ChimeraTK::VariableMapper::getInstance().getLocationAndCode();
    for(const auto& loc : ChimeraTK::VariableMapper::getInstance().getAllLocations()) {
      int codeToSet = 10;
      bool defaultUsed = true;
      // if no code is set in xml file, 10 is default
      if(locMap.find(loc) != locMap.end()) {
        codeToSet = locMap.find(loc)->second;
        defaultUsed = false;
      }

      auto* eq = find_device(loc);
      if(eq == nullptr) {
        add_location(codeToSet, loc);
      }
      else if(eq->fct_code() != codeToSet && !defaultUsed) {
        throw ChimeraTK::logic_error("Location '" + loc + "' has already a fct_code '" +
            std::to_string(eq->fct_code()) + "' from config file, that does not match with '" +
            std::to_string(codeToSet) + "' from xml file");
      }
    }

    // check for variables not yet initialised - we must guarantee that all to-application variables are written exactly
    // once at server start.
    for(auto& pv : doocsAdapter.getControlSystemPVManager()->getAllProcessVariables()) {
      if(!pv->isWriteable()) {
        continue;
      }
      if(pv->getVersionNumber() == ChimeraTK::VersionNumber(nullptr)) {
        // The variable has not yet been written. Do it now, even if we just send a 0.
        pv->write();
      }
    }

    // start the application and the updater
    ChimeraTK::ApplicationBase::getInstance().run();
    doocsAdapter.updater->run();
    ChimeraTK::DoocsAdapter::isInitialised = true;
  }

  /********************************************************************************************************************/

  void DoocsAdapter::eqCancel() {
    ChimeraTK::DoocsAdapter::isInitialised = false;
  }

  /********************************************************************************************************************/

} // namespace ChimeraTK
