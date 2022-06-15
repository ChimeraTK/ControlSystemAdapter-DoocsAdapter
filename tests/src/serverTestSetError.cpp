#define BOOST_TEST_MODULE serverTestSetError

#include <boost/test/included/unit_test.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include <eq_fct.h>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

std::string lastErrString = "(unset)";

// basic check for error code handling, without associated message source
BOOST_AUTO_TEST_CASE(testErrorNoMessageSource) {
  auto check = [] {
    auto location = getLocationFromPropertyAddress("//INT/FROM_DEVICE_SCALAR");
    location->lock();
    int code = location->get_error();
    lastErrString = location->get_errorstr();
    location->unlock();

    return static_cast<eq_errors>(code);
  };
  // Testing initial state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 0);

  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);
  BOOST_CHECK(lastErrString == "ok");

  // Check fault state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::faulty;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 1);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, not_available);
  // there must be a generic error message
  BOOST_CHECK(lastErrString != "ok");
}
