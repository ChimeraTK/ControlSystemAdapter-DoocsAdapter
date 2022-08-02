// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsAdapter.h"
#include "getAllVariableNames.h"
#include "PropertyDescription.h"
#include "VariableMapper.h"
#include <sys/stat.h>

#include <ChimeraTK/ControlSystemAdapter/ApplicationBase.h>

char const* object_name;
static char const* XML_CONFIG_SUFFIX = "-DoocsVariableConfig.xml";

ChimeraTK::DoocsAdapter doocsAdapter;

extern int eq_server(int, char**);

int main(int argc, char* argv[]) {
  object_name = ChimeraTK::ApplicationBase::getInstance().getName().c_str();
  return eq_server(argc, argv);
}

/* eq_init_prolog is called before the locations are created, i.e. before the
 * first call to eq_create. We initialise the application, i.e. all process
 * variables are created in this function. */
void eq_init_prolog() {
  // set the DOOCS server name to the application name
  // Create static instances for all applications cores. They must not have
  // overlapping process variable names ("location/protery" must be unique).
  ChimeraTK::ApplicationBase::getInstance().setPVManager(doocsAdapter.getDevicePVManager());
  ChimeraTK::ApplicationBase::getInstance().initialise();

  // the variable manager can only be filled after we have the CS manager
  auto pvNames = ChimeraTK::getAllVariableNames(doocsAdapter.getControlSystemPVManager());

  auto xmlFileName = ChimeraTK::ApplicationBase::getInstance().getName() + XML_CONFIG_SUFFIX;

  struct stat buffer;
  if(stat(xmlFileName.c_str(), &buffer) == 0) {
    ChimeraTK::VariableMapper::getInstance().prepareOutput(xmlFileName, pvNames);
  }
  else {
    std::cerr << "WARNING: No XML file for the Doocs variable config found. Trying direct import." << std::endl;
    ChimeraTK::VariableMapper::getInstance().directImport(pvNames);
  }

  // prepare list of unmapped read variables and pass it to the Application for optimisation
  for(auto &p : ChimeraTK::VariableMapper::getInstance().getUsedVariables()) {
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
        if(attr->isWriteable) ++writeableCount;
      }
      if(writeableCount == 0) {
        continue;
      }
      if(writeableCount > 1) {
        // This case is not (yet) covered. It would require some synchronisation mechanism between writeable properties,
        // which is not trivial as inconsistencies and dead locks must be avoided.
        std::cout << "**** WARNING: Variable '" + p.first +
                "' mapped to more than one writeable property. Expect inconsistencies!\n";
      }

      // Add PVs which are used at least twice with at least one writable property to list
      doocsAdapter.writeableVariablesWithMultipleProperties[p.first] = {};
    }
  }
}

/* eq_create returns a ControlSystemAdapter-based location for any location type
 */
EqFct* eq_create(int eq_code, void*) {
  return new ChimeraTK::CSAdapterEqFct(eq_code, doocsAdapter.getControlSystemPVManager(), doocsAdapter.updater);
}

/* post_init_epilog is called after all DOOCS properties are fully intialised,
 * including any value intialisation from the config file. We start the
 * application here. It will be launched in a separate thread. */
void post_init_epilog() {
  // check for locations not yet created (due to missing entries in the .conf file) and create them now
  std::map<std::string, int> locMap = ChimeraTK::VariableMapper::getInstance().getLocationAndCode();
  for(auto& loc : ChimeraTK::VariableMapper::getInstance().getAllLocations()) {
    int codeToSet = 10;
    bool defaultUsed = true;
    // if no code is set in xml file, 10 is default
    if(locMap.find(loc) != locMap.end()) {
      codeToSet = locMap.find(loc)->second;
      defaultUsed = false;
    }

    auto eq = find_device(loc);
    if(eq == nullptr) {
      add_location(codeToSet, loc);
    }
    else if(eq->fct_code() != codeToSet && !defaultUsed) {
      throw ChimeraTK::logic_error("Location '" + loc + "' has already a fct_code '" + std::to_string(eq->fct_code()) +
          "' from config file, that does not match with '" + std::to_string(codeToSet) + "' from xml file");
    }
  }

  // check for variables not yet initialised - we must guarantee that all to-application variables are written exactly
  // once at server start.
  for(auto& pv : doocsAdapter.getControlSystemPVManager()->getAllProcessVariables()) {
    if(!pv->isWriteable()) continue;
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

void eq_cancel() {
  ChimeraTK::DoocsAdapter::isInitialised = false;
}
