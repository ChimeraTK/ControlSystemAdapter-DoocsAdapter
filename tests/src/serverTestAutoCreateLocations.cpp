#define BOOST_TEST_MODULE serverTestAutoCreateLocations

#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <boost/filesystem.hpp>

#include "DoocsAdapter.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <thread>

using namespace boost::unit_test;
using namespace boost::unit_test_framework;
using namespace ChimeraTK;

struct InternalTestServer : public ThreadedDoocsServer {
  InternalTestServer(const std::string& serverName) : ThreadedDoocsServer({}, 0, nullptr, false) {
    _serverName = serverName;
  }
};

struct GlobalFixture {
  GlobalFixture() {
    boost::filesystem::copy_file(framework::master_test_suite().p_name.value + ".template.conf",
        framework::master_test_suite().p_name.value + ".conf", boost::filesystem::copy_option::overwrite_if_exists);
    server.start(framework::master_test_suite().argc, framework::master_test_suite().argv);
    ChimeraTK::DoocsAdapter::waitUntilInitialised();
  }

  ReferenceTestApplication referenceTestApplication{framework::master_test_suite().p_name.value};
  InternalTestServer server{framework::master_test_suite().p_name.value};
};

#if defined(BOOST_FIXTURE_NEEDS_SEMICOLON)
BOOST_GLOBAL_FIXTURE(GlobalFixture);
#else
BOOST_GLOBAL_FIXTURE(GlobalFixture)
#endif

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
