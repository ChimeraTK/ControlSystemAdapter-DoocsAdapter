// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestVariableMapperWithLocationAndCode
#include <boost/test/included/unit_test.hpp>

#include <boost/filesystem.hpp>

// #include <boost/test/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <thread>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE

BOOST_AUTO_TEST_CASE(testCodeIsSetCorrectly) {
  EqFct* eq = find_device("CREATED");
  BOOST_CHECK_EQUAL(eq->fct_code(), 10);

  eq = find_device("NEW");
  BOOST_CHECK_EQUAL(eq->fct_code(), 12);

  eq = find_device("DOUBLE");
  BOOST_CHECK_EQUAL(eq->fct_code(), 10);

  eq = find_device("FLOAT");
  BOOST_CHECK_EQUAL(eq->fct_code(), 11);
}
