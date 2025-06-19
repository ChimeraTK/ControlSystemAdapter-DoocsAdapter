// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestDataMatching

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

/**********************************************************************************************************************/

struct DataMatchingFixture {
  // start values for sending
  int macroPulseNumber = 1000;
  unsigned sendUnsigned = 40U;
  float sendFloat = 40.1F;
  double sendDouble = 40.2;
  // the expectation whether sent updates should propagate back
  bool expectUnsignedUpdated = true;
  bool expectFloatUpdated = true;
  bool expectDoubleUpdated = true;
  // internal storage for next check
  unsigned expectedUnsigned;
  float expectedFloat;
  double expectedDouble;

  void sendUpdate(bool updateMacroPulse = true, bool updateValues = true, ChimeraTK::VersionNumber vn = {}) {
    // We need data consistency between macro pulse number and the data
    // Everything from now on will get the same version number, until we manually set a new one.
    GlobalFixture::referenceTestApplication.versionNumber = vn;

    if(updateMacroPulse) {
      macroPulseNumber++;
      DoocsServerTestHelper::doocsSet<int>("//UNMAPPED/INT.TO_DEVICE_SCALAR", macroPulseNumber);
    }
    if(updateValues) {
      ++sendUnsigned;
      ++sendFloat;
      ++sendDouble;
      DoocsServerTestHelper::doocsSet<unsigned int>("//UINT/TO_DEVICE_SCALAR", sendUnsigned);
      DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_SCALAR", sendFloat);
      DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_SCALAR", sendDouble);
      // save values for next check, according to known expectation
      if(expectUnsignedUpdated) {
        expectedUnsigned = sendUnsigned;
      }
      if(expectFloatUpdated) {
        expectedFloat = sendFloat;
      }
      if(expectDoubleUpdated) {
        expectedDouble = sendDouble;
      }
    }
  }

  void checkReceivedValues() {
    if(!expectUnsignedUpdated or !expectFloatUpdated or !expectDoubleUpdated) {
      usleep(1000000);
    }

    TEST_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<unsigned int>("//UINT/FROM_DEVICE_SCALAR") == expectedUnsigned);
    TEST_WITH_TIMEOUT(
        std::abs(DoocsServerTestHelper::doocsGet<float>("//FLOAT/FROM_DEVICE_SCALAR") - expectedFloat) < 1e-6);
    // note about precision: doocsGet uses get_float so test not stricter than for float
    TEST_WITH_TIMEOUT(
        std::abs(DoocsServerTestHelper::doocsGet<double>("//DOUBLE/FROM_DEVICE_SCALAR") - expectedDouble) < 1e-6);
  }
};

BOOST_FIXTURE_TEST_CASE(testProcessScalar, DataMatchingFixture) {
  std::cout << "testProcessScalar" << std::endl;

  sendUpdate();
  ExtendedTestApplication::runMainLoopOnce();
  checkReceivedValues();

  // Send MPN and values with different version numbers
  // Unsigned int value must not be updated because it has data matching exact
  expectUnsignedUpdated = false;
  // Float value has to get the update even with version number mismatch because data matching is none
  expectFloatUpdated = true;
  // double value must also not be updated because data matching = historized
  expectDoubleUpdated = false;

  sendUpdate(false);
  ExtendedTestApplication::runMainLoopOnce();
  checkReceivedValues();

  // Another iteration, now back to consistent version number
  expectUnsignedUpdated = true;
  expectDoubleUpdated = true;

  sendUpdate();
  ExtendedTestApplication::runMainLoopOnce();
  checkReceivedValues();

  // check that after value only updates, historized update catches up when finally matching versionNo is available in
  // macro pulse
  unsigned uValOld = sendUnsigned;
  ChimeraTK::VersionNumber vnFirst;
  sendUpdate(false, true, vnFirst);
  double dValFirst = sendDouble;
  ExtendedTestApplication::runMainLoopOnce();
  sendUpdate(false, true, {});
  ExtendedTestApplication::runMainLoopOnce();
  sendUpdate(true, false, vnFirst);
  ExtendedTestApplication::runMainLoopOnce();
  // we expect old non-updated value for unsigned (since was not matching exactly),
  expectedUnsigned = uValOld;
  // expect most recent float value,
  expectedFloat = sendFloat;
  // and expect first double val because macro pulse has catched up
  expectedDouble = dValFirst;
  checkReceivedValues();
}

/**********************************************************************************************************************/
