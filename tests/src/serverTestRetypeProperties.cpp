#define BOOST_TEST_MODULE serverTestRetypeProperties
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(testVariableExistence) {
  checkDoocsProperty<D_int>("//TO_BYTE/DOUBLE.CONSTANT", true, false);
  checkDoocsProperty<D_int>("//TO_SHORT/FLOAT.CONSTANT", true, false);
  checkDoocsProperty<D_int>("//TO_INT/SHORT.CONSTANT", true, false);
  checkDoocsProperty<D_longarray>("//TO_LONG/UCHAR.CONSTANT", true, false);
  checkDoocsProperty<D_float>("//TO_FLOAT/INT.CONSTANT", true, false);
  checkDoocsProperty<D_double>("//TO_DOUBLE/USHORT.CONSTANT", true, false);
}
