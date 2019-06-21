#define BOOST_TEST_MODULE serverTestErrorFlag

#include <boost/test/included/unit_test.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include <eq_fct.h>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

template<typename DOOCS_TYPE>
static eq_errors checkErrorFlag(const std::string& property) {
  auto location = getLocationFromPropertyAddress(property);
  location->lock();
  auto p = getDoocsProperty<DOOCS_TYPE>(property);
  auto value = static_cast<eq_errors>(p->d_error());
  location->unlock();

  return value;
}

BOOST_AUTO_TEST_CASE(testScalar) {
  auto check = std::bind(checkErrorFlag<D_int>, "//INT/FROM_DEVICE_SCALAR");
  // Testing initial state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 12);

  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);

  // Check fault state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::faulty;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 12);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, stale_data);
}

BOOST_AUTO_TEST_CASE(testArray) {
  auto check = std::bind(checkErrorFlag<D_intarray>, "//INT/FROM_DEVICE_ARRAY");

  // Testing initial state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_ARRAY", std::initializer_list<int>{12, 12, 12});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);

  // Testing fault state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::faulty;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_ARRAY", std::initializer_list<int>{12, 12, 12});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, stale_data);

  // Test clearing the flag again
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_ARRAY", std::initializer_list<int>{12, 12, 12});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);
}

BOOST_AUTO_TEST_CASE(testSpectrum) {
  auto check = std::bind(checkErrorFlag<D_spectrum>, "//SPECTRUM/TEST");

  // Testing initial state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);

  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::faulty;
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, stale_data);

  // Testing initial state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);
}

BOOST_AUTO_TEST_CASE(testXY) {
  auto check = std::bind(checkErrorFlag<D_xy>, "//XY/TEST");

  // Testing initial state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<double>(
      "//DOUBLE/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);

  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::faulty;
  DoocsServerTestHelper::doocsSet<double>(
      "//DOUBLE/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, stale_data);

  // Testing initial state
  GlobalFixture::referenceTestApplication.dataValidity = DataValidity::ok;
  DoocsServerTestHelper::doocsSet<double>(
      "//DOUBLE/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  checkWithTimeout<eq_errors>(check, no_error);
}