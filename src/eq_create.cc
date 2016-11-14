#include <ChimeraTK/ControlSystemAdapter/ApplicationBase.h>

#include "DoocsAdapter.h"

char const *object_name;

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
