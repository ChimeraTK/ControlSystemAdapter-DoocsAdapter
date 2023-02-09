// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestZeroMQ

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

extern const char* object_name;
#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <eq_client.h>

#include <random>
#include <thread>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION_WITH_CODE(dmsg_start();)

static std::atomic<uint32_t> dataReceived;
static EqData received;
static dmsg_info_t receivedInfo;
static std::mutex mutex;

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testScalar) {
  std::cout << "testScalar " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();

  // We need data consistency between macro pulse number and the data
  // Everything from now on will get the same version number, until we manually set a new one.
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

  // set initial values
  int macroPulseNumber = 12345;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);

  uint32_t expectedValue = 42;
  // this makes sure initial value that we read back should be valid
  DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  /// Note: The data is processed by the ReferenceTestApplication in the order
  /// of the types as listed in the HolderMap of the ReferenceTestApplication.
  /// INT comes before UINT and FLOAT, so the macro pulse number is first
  /// written and then our values.
  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/UINT/FROM_DEVICE_SCALAR");
  dmsg_t tag;
  dataReceived = 0;
  int err = dmsg_attach(
      &ea, &dst, nullptr,
      [](void*, EqData* data, dmsg_info_t* info) {
        std::lock_guard<std::mutex> lock(mutex);
        received.copy_from(data);
        receivedInfo = *info;
        dataReceived++;
      },
      &tag);
  BOOST_CHECK(!err);

  // Wait until initial value is received, which is polled via RPC by the DOOCS serverlib.
  // The difficulty here is that ZMQ responds already before connection was established, with
  // received.error() = unavail_serv
  // need long timeout ~ 1 minute
  for(unsigned local_index_i = 0; local_index_i < 60000; ++local_index_i) {
    usleep(1000);
    if(dataReceived > 0 && received.error() == 0) {
      std::cout << "ok after " << local_index_i << std::endl;
      break;
    }
  }
  {
    std::lock_guard<std::mutex> lock(mutex);
    BOOST_CHECK_EQUAL(received.error(), 0);
    BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
  }
  dataReceived = 0;

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  while(dataReceived == 0) {
    // First send, then wait. We assume that after 10 ms the event has been received once the ZMQ mechanism is up and
    // running
    DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    usleep(10000);
    counter++;
    BOOST_REQUIRE(counter < 1000);
  }
  {
    // check first value actually transported via ZMQ protocol
    std::lock_guard<std::mutex> lock(mutex);
    BOOST_CHECK_EQUAL(received.error(), 0);
    BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
  }

  // make sure no more data is "in the pipeline"
  while(dataReceived > 0) {
    dataReceived = 0;
    usleep(10000);
  }

  // From now on, each consistent update should be received.
  // Make sure consistent receiving is happening whether the macro pulse number is send first or second.
  std::array<bool, 10> sendMacroPulseFirst = {true, true, true, false, false, false, true, false, true, false};
  std::array<bool, 10> sendError = {false, false, false, false, false, false, false, false, true, true};
  for(size_t i = 0; i < 10; ++i) {
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
    ++macroPulseNumber;
    expectedValue = 100 + i;
    if(sendMacroPulseFirst[i]) {
      DoocsServerTestHelper::doocsSet<int32_t>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
    }
    else { // send the value first
      DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);
    }

    // nothing must be received, no consistent set yet
    BOOST_CHECK_EQUAL(dataReceived, 0);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    usleep(10000);
    BOOST_CHECK_EQUAL(dataReceived, 0);

    // now send the variable which has not been send yet
    if(sendMacroPulseFirst[i]) {
      DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);
    }
    else {
      DoocsServerTestHelper::doocsSet<int32_t>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
    }
    bool isError = sendError[i];
    if(isError) {
      GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::faulty;
    }
    else {
      GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::ok;
    }
    GlobalFixture::referenceTestApplication.runMainLoopOnce();

    CHECK_WITH_TIMEOUT(dataReceived > 0);
    usleep(10000);
    BOOST_CHECK_EQUAL(dataReceived, 1);
    dataReceived--;
    {
      std::lock_guard<std::mutex> lock(mutex);
      BOOST_CHECK_EQUAL(received.error() != 0, isError);
      BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
      auto time = appPVmanager->getProcessArray<uint32_t>("UINT/FROM_DEVICE_SCALAR")->getVersionNumber().getTime();
      auto secs = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
      auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() - secs * 1e6;
      BOOST_CHECK_EQUAL(receivedInfo.sec, secs);
      BOOST_CHECK_EQUAL(receivedInfo.usec, usecs);
      BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
      BOOST_CHECK_EQUAL(receivedInfo.stat != 0, isError);
    }
  }

  dmsg_detach(&ea, tag);
  GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::ok;
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testArray) {
  std::cout << "testArray " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();

  // We need data consistency between macro pulse number and the data
  // Everything from now on will get the same version number, until we manually set a new one.
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

  // set initial values
  int macroPulseNumber = 99999;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);

  std::vector<int32_t> expectedArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
  DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", expectedArrayValue);

  size_t retryCounter = 0;
retry:
  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/UINT/FROM_DEVICE_ARRAY");
  dmsg_t tag;
  dataReceived = 0;
  int err = dmsg_attach(
      &ea, &dst, nullptr,
      [](void*, EqData* data, dmsg_info_t* info) {
        std::lock_guard<std::mutex> lock(mutex);
        received.copy_from(data);
        receivedInfo = *info;
        dataReceived++;
      },
      &tag);
  BOOST_CHECK(!err);

  // Wait until initial value is received (which is polled via RPC by the DOOCS serverlib)
  CHECK_WITH_TIMEOUT(dataReceived > 0);
  usleep(10000);
  BOOST_CHECK_EQUAL(dataReceived, 1);
  dataReceived = 0;

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  while(dataReceived == 0) {
    DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", expectedArrayValue);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    // FIXME: This timeout is essential so everything has been received and the next
    // dataReceived really is false. It is a potential source of timing problems /
    // race conditions in this test.
    usleep(10000);
    if(++counter > 1000) {
      dmsg_detach(&ea, tag);
      std::cout << "RETRY!" << std::endl;
      ++retryCounter;
      BOOST_REQUIRE(retryCounter < 10);
      goto retry;
    }
  }
  BOOST_CHECK(dataReceived > 0);
  {
    std::lock_guard<std::mutex> lock(mutex);
    BOOST_CHECK_EQUAL(received.error(), 0);
    BOOST_CHECK_EQUAL(received.length(), 10);
    for(size_t k = 0; k < 10; ++k) BOOST_CHECK_EQUAL(received.get_int(k), expectedArrayValue[k]);
  }

  // make sure no more data is "in the pipeline"
  while(dataReceived > 0) {
    dataReceived = 0;
    usleep(10000);
  }

  // From now on, each consistent update should be received.
  // Make sure consistent receiving is happening whether the macro pulse number is send first or second.
  std::array<bool, 10> sendMacroPulseFirst = {true, true, true, false, false, false, true, false, true, false};
  for(size_t i = 0; i < 10; ++i) {
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
    --macroPulseNumber;
    expectedArrayValue[1] = 100 + i;
    if(sendMacroPulseFirst[i]) {
      DoocsServerTestHelper::doocsSet<int32_t>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
    }
    else { // send the value first
      DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", expectedArrayValue);
    }

    // nothing must be received, no consistent set yet
    BOOST_CHECK_EQUAL(dataReceived, 0);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    usleep(10000);
    BOOST_CHECK_EQUAL(dataReceived, 0);

    // now send the variable which has not been send yet
    if(sendMacroPulseFirst[i]) {
      DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", expectedArrayValue);
    }
    else {
      DoocsServerTestHelper::doocsSet<int32_t>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
    }
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived > 0);
    usleep(10000);
    BOOST_CHECK_EQUAL(dataReceived, 1);
    dataReceived--;
    {
      std::lock_guard<std::mutex> lock(mutex);
      BOOST_CHECK_EQUAL(received.error(), 0);
      BOOST_CHECK_EQUAL(received.length(), 10);
      for(size_t k = 0; k < 10; ++k) BOOST_CHECK_EQUAL(received.get_int(k), expectedArrayValue[k]);
      auto time = appPVmanager->getProcessArray<uint32_t>("UINT/FROM_DEVICE_ARRAY")->getVersionNumber().getTime();
      auto secs = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
      auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() - secs * 1e6;
      BOOST_CHECK_EQUAL(receivedInfo.sec, secs);
      BOOST_CHECK_EQUAL(receivedInfo.usec, usecs);
      BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testSpectrum) {
  std::cout << "testSpectrum " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();

  // We need data consistency between macro pulse number and the data
  // Everything from now on will get the same version number, until we manually set a new one.
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

  // set initial values
  int macroPulseNumber = -100;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);

  std::vector<float> expectedFloatArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);

  size_t retryCounter = 0;
retry:
  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/FLOAT/FROM_DEVICE_ARRAY");
  dmsg_t tag;
  dataReceived = 0;
  int err = dmsg_attach(
      &ea, &dst, nullptr,
      [](void*, EqData* data, dmsg_info_t* info) {
        std::lock_guard<std::mutex> lock(mutex);
        received.copy_from(data);
        receivedInfo = *info;
        dataReceived++;
      },
      &tag);
  BOOST_CHECK(!err);

  // Wait until initial value is received (which is polled via RPC by the DOOCS serverlib)
  CHECK_WITH_TIMEOUT(dataReceived > 0);
  usleep(10000);
  BOOST_CHECK_EQUAL(dataReceived, 1);
  dataReceived = 0;

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  while(dataReceived == 0) {
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    // FIXME: This timeout is essential so everything has been received and the next
    // dataReceived really is false. It is a potential source of timing problems /
    // race conditions in this test.
    usleep(10000);
    if(++counter > 1000) {
      dmsg_detach(&ea, tag);
      std::cout << "RETRY!" << std::endl;
      ++retryCounter;
      BOOST_REQUIRE(retryCounter < 10);
      goto retry;
    }
  }
  BOOST_CHECK(dataReceived > 0);
  // note first value via ZMQ != initial value (which would be invalid)
  BOOST_CHECK_EQUAL(received.error(), 0);
  BOOST_CHECK_EQUAL(received.length(), 10);
  BOOST_CHECK_CLOSE(received.get_spectrum()->s_start, 123., 0.001);
  BOOST_CHECK_CLOSE(received.get_spectrum()->s_inc, 0.56, 0.001);
  for(size_t k = 0; k < 10; ++k) BOOST_CHECK_CLOSE(received.get_float(k), expectedFloatArrayValue[k], 0.0001);

  // make sure no more data is "in the pipeline"
  while(dataReceived > 0) {
    dataReceived = 0;
    usleep(10000);
  }

  // From now on, each consistent update should be received.
  // Make sure consistent receiving is happening whether the macro pulse number is send first or second.
  std::array<bool, 10> sendMacroPulseFirst = {true, true, true, false, false, false, true, false, true, false};
  std::array<bool, 10> sendError = {false, false, false, false, false, false, false, false, true, true};
  for(size_t i = 0; i < 10; ++i) {
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
    macroPulseNumber *= -2;
    expectedFloatArrayValue[1] = 100 + i;
    if(sendMacroPulseFirst[i]) {
      DoocsServerTestHelper::doocsSet<int32_t>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
    }
    else { // send the value first
      DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
    }

    // nothing must be received, no consistent set yet
    BOOST_CHECK_EQUAL(dataReceived, 0);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    usleep(10000);
    BOOST_CHECK_EQUAL(dataReceived, 0);

    // now send the variable which has not been send yet
    if(sendMacroPulseFirst[i]) {
      DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
    }
    else {
      DoocsServerTestHelper::doocsSet<int32_t>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
    }
    bool isError = sendError[i];
    if(isError) {
      GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::faulty;
    }
    else {
      GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::ok;
    }
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived > 0);
    usleep(10000);
    BOOST_CHECK_EQUAL(dataReceived, 1);
    dataReceived--;
    {
      std::lock_guard<std::mutex> lock(mutex);
      BOOST_CHECK_EQUAL(received.error() != 0, isError);
      BOOST_CHECK_EQUAL(received.length(), 10);
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_start, 123., 0.001);
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_inc, 0.56, 0.001);
      for(size_t k = 0; k < 10; ++k) BOOST_CHECK_CLOSE(received.get_float(k), expectedFloatArrayValue[k], 0.0001);
      auto time = appPVmanager->getProcessArray<float>("FLOAT/FROM_DEVICE_ARRAY")->getVersionNumber().getTime();
      auto secs = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
      auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() - secs * 1e6;
      // check ZMQ header data
      BOOST_CHECK_EQUAL(receivedInfo.sec, secs);
      BOOST_CHECK_EQUAL(receivedInfo.usec, usecs);
      BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
      BOOST_CHECK_EQUAL(receivedInfo.stat != 0, isError);
      // note, the "status" field in spectrum metadata (of ZMQ payload) is not set as an error field
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/
