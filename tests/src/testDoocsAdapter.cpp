// Define a name for the test module.
#define BOOST_TEST_MODULE DoocsAdapterTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "emptyServerFunctions.h"

#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessScalar.h>
#include <ChimeraTK/ControlSystemAdapter/SynchronizationDirection.h>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

BOOST_AUTO_TEST_SUITE( DoocsAdapterRestSuite )

BOOST_AUTO_TEST_CASE( testDoocsAdapter ) {
  DoocsAdapter doocsAdapter;

  //not much to test. We can test that the shared pointers are not null,
  //that's it.
  BOOST_CHECK(doocsAdapter.getDevicePVManager());
  BOOST_CHECK(doocsAdapter.getControlSystemPVManager());
}

BOOST_AUTO_TEST_SUITE_END()
