// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestXy

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

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

const std::string PROPERTY_NAME{"//FLOAT/TEST_XY"};

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testXyMetadata) {
  std::cout << "testXyMetadata" << std::endl;

  auto* xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
  BOOST_CHECK_EQUAL(xy->description(), "Some XY test");
  BOOST_CHECK_EQUAL(xy->max_length(), 10);
  float f1, f2;
  int i1;
  time_t tmp;
  char buf[255];

  xy->plot_x_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
  BOOST_CHECK_EQUAL(buf, "mm");
  xy->plot_y_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
  BOOST_CHECK_EQUAL(buf, "µs");
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testXyUpdates) {
  std::cout << "testXy" << std::endl;

  auto check = []() -> double {
    auto* xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    auto* location = getLocationFromPropertyAddress(PROPERTY_NAME);
    double value;

    location->lock();
    value = xy->xy()->y_data;
    location->unlock();

    return value;
  };

  // Send values with inconsistent versions
  std::vector<float> xArrayValues = {0.0F, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", xArrayValues);
  std::vector<double> yArrayValues = {-10.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0F};
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);

  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // Wait a bit
  usleep(2000000);

  // Nothing should have happened
  {
    auto* xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto* val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, 0.0F, 1e-6F);
      BOOST_CHECK_CLOSE(val->y_data, 0.0F, 1e-6F);
    }
  }

  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", xArrayValues);
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  checkWithTimeout<double>(check, -10.0);

  // Nothing should have happened
  {
    auto* xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto* val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6F);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i], 1e-6F);
    }
  }

  /* Just updating Y values with matching version */
  std::transform(yArrayValues.begin(), yArrayValues.end(), yArrayValues.begin(), [](double v) { return v * 10.0; });
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  checkWithTimeout<double>(check, -10.0 * 10.0);
  // Nothing should have happened
  {
    auto* xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto* val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6F);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i], 1e-6F);
    }
  }

  std::transform(yArrayValues.begin(), yArrayValues.end(), yArrayValues.begin(), [](double v) { return v * 10.0; });

  GlobalFixture::referenceTestApplication.versionNumber = boost::none;
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // Wait a bit
  usleep(2000000);

  // Nothing should have happened
  {
    auto* xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto* val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6F);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i] / 10.0, 1e-6F);
    }
  }
}

/**********************************************************************************************************************/
