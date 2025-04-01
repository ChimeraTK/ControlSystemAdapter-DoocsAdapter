// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestSetError

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <boost/test/included/unit_test.hpp>

extern const char* object_name;
#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

#include <eq_fct.h>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

std::string lastErrString = "(unset)";

// basic check for error code handling, without associated message source
BOOST_AUTO_TEST_CASE(testErrorNoMessageSource) {
  auto check = [] {
    auto* location = getLocationFromPropertyAddress("//INT/FROM_DEVICE_SCALAR");
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
  TEST_WITH_TIMEOUT((check(), (lastErrString == "ok")));

  // Check fault state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::faulty;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 1);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, not_available);
  // there must be a generic error message
  TEST_WITH_TIMEOUT((check(), (lastErrString != "ok")));
}

BOOST_AUTO_TEST_CASE(setErrorSourceMarkedAsUsed) {
  // kind of a regression test for bug #11853
  // (set_error source was thrown away in optimization step since it was not marked as used)
  // - do not explicitly map statusCodeSource of set_error
  // - check that statusCodeSource goes into set of used variables
  bool res = GlobalFixture::referenceTestApplication.unmappedVariables.find("/INT/FROM_DEVICE_SCALAR") ==
      GlobalFixture::referenceTestApplication.unmappedVariables.end();
  BOOST_TEST(res);
}
