// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestIiii

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

extern const char* object_name;
#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <eq_client.h>

#include <algorithm>
#include <random>
#include <thread>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

const std::string PROPERTY_NAME{"//CUSTOM/IIII"};

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

/**********************************************************************************************************************/
BOOST_AUTO_TEST_CASE(testIiiiUpdate) {
  std::cout << "testIiiiUpdate" << std::endl;

  auto extractValue = []() -> IIII {
    auto d_iiii = getDoocsProperty<D_iiii>(PROPERTY_NAME);
    auto location = getLocationFromPropertyAddress(PROPERTY_NAME);
    IIII value; // a copy of the value. We don't want to hold the location lock longer than needed

    location->lock();
    value = *(
        d_iiii
            ->value()); // value returs a pointer which we only must dereference while holding the lock. So we make a copy
    location->unlock();
    return value;
  };

  auto writeIiii = [&](IIII iiii) {
    // we have to use the names if the correct variables
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
    DoocsServerTestHelper::doocsSet<int>("//IIII/TO_DEVICE", {iiii.i1_data, iiii.i2_data, iiii.i3_data, iiii.i4_data});
    ReferenceTestApplication::runMainLoopOnce();
  };

  IIII referenceIiii;
  referenceIiii.i1_data = 123;
  referenceIiii.i2_data = 12;
  referenceIiii.i3_data = -12;
  referenceIiii.i4_data = -123;

  writeIiii(referenceIiii);

  IIII resultIiii;
  checkWithTimeout<int>(
      [&]() -> int {
        resultIiii = extractValue();
        return resultIiii.i1_data;
      },
      referenceIiii.i1_data);
  BOOST_TEST(resultIiii.i2_data == referenceIiii.i2_data);
  BOOST_TEST(resultIiii.i3_data == referenceIiii.i3_data);
  BOOST_TEST(resultIiii.i4_data == referenceIiii.i4_data);

  // change the values
  referenceIiii.i1_data = 234;
  referenceIiii.i2_data = 23;
  referenceIiii.i3_data = -23;
  referenceIiii.i4_data = -234;

  writeIiii(referenceIiii);

  checkWithTimeout<int>(
      [&]() -> int {
        resultIiii = extractValue();
        return resultIiii.i1_data;
      },
      referenceIiii.i1_data);
  BOOST_TEST(resultIiii.i2_data == referenceIiii.i2_data);
  BOOST_TEST(resultIiii.i3_data == referenceIiii.i3_data);
  BOOST_TEST(resultIiii.i4_data == referenceIiii.i4_data);

  // change the values
  IIII newReferenceIiii;
  newReferenceIiii.i1_data = 345;
  newReferenceIiii.i2_data = 34;
  newReferenceIiii.i3_data = -34;
  newReferenceIiii.i4_data = -345;
}

/**********************************************************************************************************************/
