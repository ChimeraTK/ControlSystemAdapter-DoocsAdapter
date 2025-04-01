// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestSpectrumBuffer

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;

#include "fixMissingDoocs_eq_res.h"

#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <doocs/EqCall.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testSpectrum) {
  std::cout << "testSpectrum" << std::endl;

  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/FLOAT/FROM_DEVICE_ARRAY");

  auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();

  int firstMacroPulseNumber = 648583; // just some random number to start with

  // starting value for the array
  std::vector<float> expectedFloatArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

  // fill 10 updates (we have 32 buffers)
  for(size_t i = 0; i < 10; ++i) {
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", firstMacroPulseNumber + i);
    expectedFloatArrayValue[1] = i;
    DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
    GlobalFixture::referenceTestApplication.runMainLoopOnce();

    // NOTE: There is a buffer overflow problem here; The loop sets
    // valued too fast for the doocsadapter to consume. The sleep is a
    // workaround to provide the doocs adapter time to digest set
    // values. A proper fix is for the referenceTestApplication to do
    // the rate limiting internally.
    sleep(1);
  }

  // read out the 10 updates from the buffers
  for(size_t i = 0; i < 10; ++i) {
    doocs::EqData src, dst;
    src.init();
    IIII par;
    par.i1_data = int(firstMacroPulseNumber + i);
    par.i2_data = 0;
    par.i3_data = 1;
    par.i4_data = 0;
    src.length(4);
    src.set(&par);
    EqCall call;
    auto rc = call.get(&ea, &src, &dst);
    if(rc != fixme::comp_code::ok) {
      BOOST_FAIL("Error connecting to DOOCS server: " + dst.get_string());
      return;
    }
    BOOST_CHECK_EQUAL(dst.error(), 0);
    expectedFloatArrayValue[1] = i;
    for(size_t k = 0; k < expectedFloatArrayValue.size(); ++k) {
      BOOST_CHECK_CLOSE(dst.get_float(k), expectedFloatArrayValue[k], 1e-6);
    }
  }

  // fill 32 updates (we have 32 buffers)
  for(size_t i = 0; i < 32; ++i) {
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", firstMacroPulseNumber + i + 10);
    expectedFloatArrayValue[1] = i + 10000;
    DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    // NOTE: There is a buffer overflow problem here; The loop sets
    // valued too fast for the doocsadapter to consume. The sleep is a
    // workaround to provide the doocs adapter time to digest set
    // values. A proper fix is for the referenceTestApplication to do
    // the rate limiting internally.
    sleep(1);
  }

  // read out the 32 updates from the buffers
  for(size_t i = 0; i < 32; ++i) {
    doocs::EqData src, dst;
    src.init();
    IIII par;
    par.i1_data = int(firstMacroPulseNumber + i + 10);
    par.i2_data = 0;
    par.i3_data = 1;
    par.i4_data = 0;
    src.length(4);
    src.set(&par);
    EqCall call;
    auto rc = call.get(&ea, &src, &dst);
    BOOST_CHECK_EQUAL(rc, fixme::comp_code::ok);
    BOOST_CHECK_EQUAL(dst.error(), 0);
    expectedFloatArrayValue[1] = i + 10000;
    for(size_t k = 0; k < expectedFloatArrayValue.size(); ++k) {
      BOOST_CHECK_CLOSE(dst.get_float(k), expectedFloatArrayValue[k], 1e-6);
    }
  }

  // try requesting macropuse with is just out of range
  {
    doocs::EqData src, dst;
    src.init();
    IIII par;
    par.i1_data = int(firstMacroPulseNumber + 9);
    par.i2_data = 0;
    par.i3_data = 1;
    par.i4_data = 0;
    src.length(4);
    src.set(&par);
    EqCall call;
    auto rc = call.get(&ea, &src, &dst);
    BOOST_CHECK_EQUAL(rc, fixme::comp_code::data_error);
    BOOST_CHECK_EQUAL(dst.error(), scope_out_of_range);
  }

  // read out the 32 updates from the buffers again
  for(size_t i = 0; i < 32; ++i) {
    doocs::EqData src, dst;
    src.init();
    IIII par;
    par.i1_data = int(firstMacroPulseNumber + i + 10);
    par.i2_data = 0;
    par.i3_data = 1;
    par.i4_data = 0;
    src.length(4);
    src.set(&par);
    EqCall call;
    auto rc = call.get(&ea, &src, &dst);
    BOOST_CHECK_EQUAL(rc, fixme::comp_code::ok);
    BOOST_CHECK_EQUAL(dst.error(), 0);
    expectedFloatArrayValue[1] = i + 10000;
    for(size_t k = 0; k < expectedFloatArrayValue.size(); ++k) {
      BOOST_CHECK_CLOSE(dst.get_float(k), expectedFloatArrayValue[k], 1e-6);
    }
  }
}

/**********************************************************************************************************************/
