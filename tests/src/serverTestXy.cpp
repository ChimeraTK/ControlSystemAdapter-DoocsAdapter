// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestXy
#include <boost/test/included/unit_test.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

#include <eq_client.h>
#include <random>
#include <thread>
#include <algorithm>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

const std::string PROPERTY_NAME{"//FLOAT/TEST_XY"};

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testXyMetadata) {
  std::cout << "testXyMetadata" << std::endl;

  auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
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
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    auto location = getLocationFromPropertyAddress(PROPERTY_NAME);
    double value;

    location->lock();
    value = xy->xy()->y_data;
    location->unlock();

    return value;
  };

  // Send values with inconsistent versions
  std::vector<float> xArrayValues = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", xArrayValues);
  std::vector<double> yArrayValues = {-10.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0f};
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);

  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // Wait a bit
  usleep(2000000);

  // Nothing should have happened
  {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, 0.0f, 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, 0.0f, 1e-6f);
    }
  }

  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", xArrayValues);
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  checkWithTimeout<double>(check, -10.0);

  // Nothing should have happened
  {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i], 1e-6f);
    }
  }

  /* Just updating Y values with matching version */
  std::transform(yArrayValues.begin(), yArrayValues.end(), yArrayValues.begin(), [](double v) { return v * 10.0; });
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  checkWithTimeout<double>(check, -10.0 * 10.0);
  // Nothing should have happened
  {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i], 1e-6f);
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
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i] / 10.0, 1e-6f);
    }
  }
}

/**********************************************************************************************************************/
