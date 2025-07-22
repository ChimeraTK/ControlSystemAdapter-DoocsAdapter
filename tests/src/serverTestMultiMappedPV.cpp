// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestMultiMappedPV

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

/**********************************************************************************************************************/

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(TestMultiMapped, *boost::unit_test::tolerance(0.0001)) {
  // Note: In the .conf file, SECOND/SCALAR_1 is initialised with 23.456, which must be ignored.
  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//FIRST/SCALAR_1") == 12.345);
  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//SECOND/SCALAR_1") == 12.345);

  DoocsServerTestHelper::doocsSet<float>("//FIRST/SCALAR_1", 42.42);

  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//FIRST/SCALAR_1") == 42.42);
  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//SECOND/SCALAR_1") == 42.42);
}

/**********************************************************************************************************************/

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(TestMultiMappedBidirectional, *boost::unit_test::tolerance(0.0001)) {
  // Note: In the .conf file, SECOND/BIDIRECTIONAL is initialised with 23.456, which must be ignored.
  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//FIRST/BIDIRECTIONAL") == 12.345);
  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//SECOND/BIDIRECTIONAL") == 12.345);

  DoocsServerTestHelper::doocsSet<float>("//FIRST/BIDIRECTIONAL", 42.42);

  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//FIRST/BIDIRECTIONAL") == 42.42);
  BOOST_TEST(DoocsServerTestHelper::doocsGet<float>("//SECOND/BIDIRECTIONAL") == 42.42);
}

/**********************************************************************************************************************/
