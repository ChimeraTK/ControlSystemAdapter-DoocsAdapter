#include <DoocsAdapter.h>

// Include all the control applications you want in this server
#include <ControlSystemAdapter/testing/IndependentTestCore.h>

BEGIN_DOOCS_SERVER("ReferenceTest DOOCS server", 10)
   // Create static instances for all applications cores. They must not have overlapping
   // process variable names ("location/protery" must be unique).
static IndependentTestCore independentTestCore(doocsAdapter.getDevicePVManager(), true);
END_DOOCS_SERVER()
