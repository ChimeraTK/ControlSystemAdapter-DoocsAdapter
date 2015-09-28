#define BOOST_TEST_MODULE DoocsProcessScalarTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include "DoocsProcessScalar.h"
#include <ControlSystemAdapter/ControlSystemPVManager.h>
#include <ControlSystemAdapter/DevicePVManager.h>
#include <d_fct.h>
#include <emptyServerFunctions.h>

using namespace boost::unit_test_framework;
using namespace mtca4u;

BOOST_AUTO_TEST_SUITE( DoocsProcessScalarTestSuite )

BOOST_AUTO_TEST_CASE( toDeviceTest ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  ProcessScalar<int32_t>::SharedPtr deviceVariable =
    devManager->createProcessScalarControlSystemToDevice<int32_t>("toDeviceVariable");
  ProcessScalar<int32_t>::SharedPtr controlSystemVariable = 
    csManager->getProcessScalar<int32_t>("toDeviceVariable");
  // set the variables to 0
  *deviceVariable=0;
  *controlSystemVariable=0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<int32_t, D_int> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar.set_value(42);
  BOOST_CHECK( *controlSystemVariable == 42 );

  // receive on the device side and check that the value has arrived
  deviceVariable->receive();
  BOOST_CHECK( *controlSystemVariable == 42 );
}

BOOST_AUTO_TEST_CASE( fromDeviceTest ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  ProcessScalar<int32_t>::SharedPtr deviceVariable =
    devManager->createProcessScalarDeviceToControlSystem<int32_t>("fromDeviceVariable");
  ProcessScalar<int32_t>::SharedPtr controlSystemVariable = 
    csManager->getProcessScalar<int32_t>("fromDeviceVariable");
  // set the variables to 0
  *deviceVariable=0;
  *controlSystemVariable=0;

  // initialise the doocs scalar
  DoocsProcessScalar<int32_t, D_int> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar.set_value(0);

  *deviceVariable=42;
  deviceVariable->send();

  BOOST_CHECK( *controlSystemVariable == 0 );
  BOOST_CHECK( doocsScalar.value() == 0 );

  syncUtil.receiveAll();
  BOOST_CHECK( *controlSystemVariable == 42 );
  BOOST_CHECK( doocsScalar.value() == 42 );
}

BOOST_AUTO_TEST_SUITE_END()
