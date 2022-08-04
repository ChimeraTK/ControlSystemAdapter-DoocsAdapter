// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestReadWrite

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(testReadWrite) {
  // just a few tests before we start
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//INT/DATA_TYPE_CONSTANT") == -4);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//CHAR/DATA_TYPE_CONSTANT") == -1);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == 0);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//CHAR/FROM_DEVICE_SCALAR") == 0);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<std::string>("//STRING/DATA_TYPE_CONSTANT") == "42.000000");

  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 42);
  DoocsServerTestHelper::doocsSet<int>("//CHAR/TO_DEVICE_SCALAR", 44);
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  DoocsServerTestHelper::doocsSet<std::string>("//STRING/TO_DEVICE_SCALAR", "fortytwo");

  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == 0);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//CHAR/FROM_DEVICE_SCALAR") == 0);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<std::string>("//STRING/FROM_DEVICE_SCALAR") == "");

  // run the application loop. Still no changes until we run the doocs server
  // update
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // now finally after the next update we should see the new data in doocs
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == 42);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//CHAR/FROM_DEVICE_SCALAR") == 44);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<std::string>("//STRING/FROM_DEVICE_SCALAR") == "fortytwo");

  // we have to get stuff as float in order to work with spectra
  auto intArray = DoocsServerTestHelper::doocsGetArray<float>("//INT/FROM_DEVICE_ARRAY");
  int testVal = 140;
  for(size_t i = 0; i < intArray.size(); ++i) {
    CHECK_WITH_TIMEOUT(
        std::fabs(DoocsServerTestHelper::doocsGetArray<float>("//INT/FROM_DEVICE_ARRAY")[i] - testVal) < 0.001);
    // can't increment in check with timeout because it evaluated the equation
    // multiple times, so it increments multiple times
    ++testVal;
  }

  auto constArray = DoocsServerTestHelper::doocsGetArray<float>("//INT/CONSTANT_ARRAY");
  for(int i = 0; i < int(constArray.size()); ++i) {
    CHECK_WITH_TIMEOUT(std::fabs(DoocsServerTestHelper::doocsGetArray<float>("//INT/CONSTANT_ARRAY")[i] -
                           (-4 * i * i)) < 0.001); // float check to compensate binary roundings errors
  }

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
