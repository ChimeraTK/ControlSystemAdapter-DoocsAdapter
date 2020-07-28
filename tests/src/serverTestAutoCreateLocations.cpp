#define BOOST_TEST_MODULE serverTestAutoCreateLocations

#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include "serverBasedTestTools.h"

#include <boost/filesystem.hpp>
#include <boost/version.hpp>

#include "DoocsAdapter.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <thread>

using namespace boost::unit_test;
using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(testVariableExistence) {
  for(auto const location : {"CHAR", "DOUBLE", "FLOAT", "INT", "SHORT", "UCHAR", "UINT", "USHORT"}) {
    for(auto const property : {"CONSTANT_ARRAY", "FROM_DEVICE_ARRAY", "TO_DEVICE_ARRAY "}) {
      // if this throws the property does not exist. we should always be able to
      // read"
      BOOST_CHECK_NO_THROW(
          DoocsServerTestHelper::doocsGetArray<int>((std::string("//") + location + "/" + property).c_str()));
    }
    for(auto const property : {"DATA_TYPE_CONSTANT", "FROM_DEVICE_SCALAR", "TO_DEVICE_SCALAR"}) {
      // if this throws the property does not exist. we should always be able to
      // read"
      BOOST_CHECK_NO_THROW(
          DoocsServerTestHelper::doocsGet<int>((std::string("//") + location + "/" + property).c_str()));
    }
  }
}
