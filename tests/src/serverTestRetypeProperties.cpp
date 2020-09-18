#define BOOST_TEST_MODULE serverTestRetypeProperties
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
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

  checkDoocsProperty<D_int>("//TO_BYTE/INT.CONSTANT", true, false);
  checkDoocsProperty<D_int>("//TO_SHORT/INT.CONSTANT", true, false);
  checkDoocsProperty<D_int>("//TO_INT/INT.CONSTANT", true, false);
  checkDoocsProperty<D_longarray>("//TO_LONG/INT.CONSTANT", true, false);
  checkDoocsProperty<D_double>("//TO_DOUBLE/INT.CONSTANT", true, false);
}

/// Check the values of the variables
BOOST_AUTO_TEST_CASE(testVariableValues) {
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//TO_BYTE/INT.CONSTANT") == -((signed)sizeof(int)));
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//TO_SHORT/INT.CONSTANT") == -((signed)sizeof(int)));
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//TO_INT/INT.CONSTANT") == -((signed)sizeof(int)));
  CHECK_WITH_TIMEOUT(
      std::abs(DoocsServerTestHelper::doocsGet<float>("//TO_FLOAT/INT.CONSTANT") + ((signed)sizeof(int))) < 0.0001F);

  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<int>("//TO_BYTE/DOUBLE.CONSTANT") == 0); // rounded 1./sizeof(double)
  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<int>("//TO_SHORT/FLOAT.CONSTANT") == 0); // rounded 1./sizeof(float)
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//TO_INT/SHORT.CONSTANT") == -((signed)sizeof(short)));
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//TO_LONG/UCHAR.CONSTANT") == sizeof(unsigned char));
  CHECK_WITH_TIMEOUT(std::abs(DoocsServerTestHelper::doocsGet<double>("//TO_DOUBLE/USHORT.CONSTANT") -
                         sizeof(unsigned short)) < 0.0001);
}
