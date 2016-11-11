#include <ChimeraTK/ControlSystemAdapter/ApplicationBase.h>

#include "DoocsAdapter.h"

char const *object_name;

EqFct* eq_create (int eq_code, void *) {
    static ChimeraTK::DoocsAdapter doocsAdapter;

    static bool isInitialised = false;
    if(!isInitialised) {
      // set the DOOCS server name to the application name
      object_name = ChimeraTK::ApplicationBase::getInstance().getName().c_str();
      // Create static instances for all applications cores. They must not have overlapping
      // process variable names ("location/protery" must be unique).
      ChimeraTK::ApplicationBase::getInstance().setPVManager(doocsAdapter.getDevicePVManager());
      ChimeraTK::ApplicationBase::getInstance().run();
      isInitialised = true;
    }

    return new ChimeraTK::CSAdapterEqFct(eq_code, doocsAdapter.getControlSystemPVManager());
}
