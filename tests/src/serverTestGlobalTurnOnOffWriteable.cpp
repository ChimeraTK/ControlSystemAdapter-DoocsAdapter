#define BOOST_TEST_MODULE serverTestGlobalTurnOnOffWriteable

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
  // the stuff with default. We are lazy and put the integer types as we have to
  // list D_double and D_float separately anyway if we don't want to do
  // meta-programming
  for(auto& location : {"SHORT", "USHORT", "CHAR", "UCHAR", "INT", "UINT"}) {
    std::cout << "testing " << location << std::endl;
    checkDoocsProperty<D_int>(std::string("//") + location + "/DATA_TYPE_CONSTANT", true, false);
    checkDoocsProperty<D_int>(std::string("//") + location + "/FROM_DEVICE_SCALAR", true, false);
    checkDoocsProperty<D_int>(std::string("//") + location + "/TO_DEVICE_SCALAR", true, false);
  }
  for(auto& name : {"CONSTANT_ARRAY", "FROM_DEVICE_ARRAY", "TO_DEVICE_ARRAY"}) {
    checkDoocsProperty<D_intarray>(std::string("//INT/") + name, true, false);
    checkDoocsProperty<D_intarray>(std::string("//UINT/") + name, true, false);
    checkDoocsProperty<D_bytearray>(std::string("//CHAR/") + name, true, false);
    checkDoocsProperty<D_bytearray>(std::string("//UCHAR/") + name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//SHORT/") + name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//USHORT/") + name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//SHORT/") + name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//USHORT/") + name, true, false);
  }

  std::cout << "testing DOUBLE" << std::endl;
  checkDoocsProperty<D_doublearray>("//DOUBLE/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/TO_DEVICE_ARRAY", true, true);
  checkDoocsProperty<D_double>("//DOUBLE/DATA_TYPE_CONSTANT", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/TO_DEVICE_SCALAR", true, false);

  std::cout << "testing FLOAT" << std::endl;
  checkDoocsProperty<D_floatarray>("//FLOAT/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/TO_DEVICE_ARRAY", true, true);
  checkDoocsProperty<D_float>("//FLOAT/DATA_TYPE_CONSTANT", true, false);
  checkDoocsProperty<D_float>("//FLOAT/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_float>("//FLOAT/TO_DEVICE_SCALAR", true, false);
}
