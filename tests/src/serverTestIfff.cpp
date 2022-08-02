// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestIfff

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be first include
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

const std::string PROPERTY_NAME{"//CUSTOM/IFFF"};

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

/**********************************************************************************************************************/
BOOST_AUTO_TEST_CASE(testIfffUpdate) {
  std::cout << "testIfffUpdate" << std::endl;

  auto extractValue = []() -> IFFF {
    auto d_ifff = getDoocsProperty<D_ifff>(PROPERTY_NAME);
    auto location = getLocationFromPropertyAddress(PROPERTY_NAME);
    IFFF value; // a copy of the value. We don't want to hold the location lock longer than needed

    location->lock();
    value = *(
        d_ifff
            ->value()); // value returs a pointer which we only must dereference while holding the lock. So we make a copy
    location->unlock();
    return value;
  };

  auto writeIfff = [&](IFFF ifff) {
    // we have to use the names if the correct variables
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", ifff.i1_data);
    DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_SCALAR", ifff.f1_data);
    DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_SCALAR", static_cast<double>(ifff.f2_data));
    DoocsServerTestHelper::doocsSet<int>("//SHORT/TO_DEVICE_SCALAR", static_cast<int>(ifff.f3_data));
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
  };

  // we used SHORT/FROM_DEVICE_SCALAR for the f3_value, so only values that fit in int16_t can be used for it, no
  // franctional numbers
  IFFF referenceIfff;
  referenceIfff.i1_data = 123;
  referenceIfff.f1_data = 0.123f;
  referenceIfff.f2_data = 12.3f;
  referenceIfff.f3_data = -123;

  writeIfff(referenceIfff);

  IFFF resultIfff;
  checkWithTimeout<int>(
      [&]() -> int {
        resultIfff = extractValue();
        return resultIfff.i1_data;
      },
      referenceIfff.i1_data);
  BOOST_CHECK_CLOSE(resultIfff.f1_data, referenceIfff.f1_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f2_data, referenceIfff.f2_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f3_data, referenceIfff.f3_data, 0.0001);

  // change the values
  referenceIfff.i1_data = 234;
  referenceIfff.f1_data = 0.234f;
  referenceIfff.f2_data = 2.34f;
  referenceIfff.f3_data = -234;

  writeIfff(referenceIfff);

  checkWithTimeout<int>(
      [&]() -> int {
        resultIfff = extractValue();
        return resultIfff.i1_data;
      },
      referenceIfff.i1_data);
  BOOST_CHECK_CLOSE(resultIfff.f1_data, referenceIfff.f1_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f2_data, referenceIfff.f2_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f3_data, referenceIfff.f3_data, 0.0001);

  // change the values
  IFFF newReferenceIfff;
  newReferenceIfff.i1_data = 345;
  newReferenceIfff.f1_data = 0.345f;
  newReferenceIfff.f2_data = 3.454f;
  newReferenceIfff.f3_data = -345;

  // check data consistency implementation
  // send only one variable, then change the version number again
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 543); // will never be seen
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  // sleep a bit, we don't except a change
  sleep(1);
  resultIfff = extractValue();
  // compare with the old reference
  BOOST_CHECK_EQUAL(resultIfff.i1_data, referenceIfff.i1_data);
  BOOST_CHECK_CLOSE(resultIfff.f1_data, referenceIfff.f1_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f2_data, referenceIfff.f2_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f3_data, referenceIfff.f3_data, 0.0001);

  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_SCALAR", newReferenceIfff.f1_data);
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_SCALAR", static_cast<double>(newReferenceIfff.f2_data));
  DoocsServerTestHelper::doocsSet<int>("//SHORT/TO_DEVICE_SCALAR", static_cast<int>(newReferenceIfff.f3_data));
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // all variables have been updated, but not in a consistent way. still no change
  sleep(1);
  resultIfff = extractValue();
  // compare with the old reference
  BOOST_CHECK_EQUAL(resultIfff.i1_data, referenceIfff.i1_data);
  BOOST_CHECK_CLOSE(resultIfff.f1_data, referenceIfff.f1_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f2_data, referenceIfff.f2_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f3_data, referenceIfff.f3_data, 0.0001);

  // finally upate the missing variable with the same version as the others
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", newReferenceIfff.i1_data);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // now we see the new reference
  checkWithTimeout<int>(
      [&]() -> int {
        resultIfff = extractValue();
        return resultIfff.i1_data;
      },
      newReferenceIfff.i1_data);
  BOOST_CHECK_CLOSE(resultIfff.f1_data, newReferenceIfff.f1_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f2_data, newReferenceIfff.f2_data, 0.0001);
  BOOST_CHECK_CLOSE(resultIfff.f3_data, newReferenceIfff.f3_data, 0.0001);
}

/**********************************************************************************************************************/
