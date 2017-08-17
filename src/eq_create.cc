#include <ChimeraTK/ControlSystemAdapter/ApplicationBase.h>

#include "DoocsAdapter.h"
#include "VariableMapper.h"
#include "getAllVariableNames.h"
#include <sys/stat.h>

char const *object_name;
static char const * DOOCS_VARIABLE_CONFIG_FILE = "DoocsVariableConfig.xml";

static ChimeraTK::DoocsAdapter doocsAdapter;

/* eq_init_prolog is called before the locations are created, i.e. before the first call to eq_create.
 * We initialise the application, i.e. all process variables are created in this function. */
void eq_init_prolog() {
    // set the DOOCS server name to the application name
    object_name = ChimeraTK::ApplicationBase::getInstance().getName().c_str();
    // Create static instances for all applications cores. They must not have overlapping
    // process variable names ("location/protery" must be unique).
    ChimeraTK::ApplicationBase::getInstance().setPVManager(doocsAdapter.getDevicePVManager());
    ChimeraTK::ApplicationBase::getInstance().initialise();

    // the variable manager can only be filled after we have the CS manager
    auto pvNames = ChimeraTK::getAllVariableNames( doocsAdapter.getControlSystemPVManager() );

    struct stat buffer;
    if (stat (DOOCS_VARIABLE_CONFIG_FILE, &buffer) == 0){ 
      ChimeraTK::VariableMapper::getInstance().prepareOutput(DOOCS_VARIABLE_CONFIG_FILE, pvNames);
    }else{
      std::cerr << "WARNIUNG: No XML file for the Doocs variable config found. Trying direct import." << std::endl;
      ChimeraTK::VariableMapper::getInstance().directImport(pvNames);
    }
    std::cout << "here is the mappging:" << std::endl;
    for (auto &tmp : ChimeraTK::VariableMapper::getInstance().getAllProperties() ){
      std::cout << tmp.first << " -> " << tmp.second.location << " / " << tmp.second.name << std::endl;
    }
    
    // activate the advanced archiver to have histories
    set_arch_mode(1);
}

/* eq_create returns a ControlSystemAdapter-based location for any location type */
EqFct* eq_create (int eq_code, void *) {
  return new ChimeraTK::CSAdapterEqFct(eq_code, doocsAdapter.getControlSystemPVManager());
}

/* post_init_epilog is called after all DOOCS properties are fully intialised, including any value intialisation from
 * the config file. We start the application here. It will be launched in a separate thread. */
void post_init_epilog() {
  ChimeraTK::ApplicationBase::getInstance().run();
}
