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

BOOST_AUTO_TEST_SUITE(ZeroMQTestSuite)

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION_WITH_CODE(dmsg_start();)

template<typename ExpectedValueType>
struct ZMQFixture {
  std::atomic<uint32_t> dataReceived;
  EqData received;
  dmsg_info_t receivedInfo;
  std::mutex mutex;

  EqData dst;
  EqAdr ea;

  dmsg_t tag;
  std::string toDevicePath;
  std::string fromDevicePath;
  uint32_t macroPulseNumber = 1;
  void init() {
    // We need data consistency between macro pulse number and the data
    // Everything from now on will get the same version number, until we manually set a new one.
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

    // set initial values
    // we use the UINT variable for the macropulse number because we want INT to be reserved for actual values
    DoocsServerTestHelper::doocsSet<int>("//UINT/TO_DEVICE_SCALAR", macroPulseNumber);

    // this makes sure initial value that we read back should be valid
    ExpectedValueType expectedValue = generateValue(0);
    setDoocsValue(expectedValue);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();

    /// Note: The data is processed by the ReferenceTestApplication in the order
    /// of the types as listed in the HolderMap of the ReferenceTestApplication.
    ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + fromDevicePath);
    dataReceived = 0;
    auto fun = [](void* fixture, EqData* data, dmsg_info_t* info) {
      auto* f = static_cast<ZMQFixture*>(fixture);
      std::lock_guard<std::mutex> lock(f->mutex);
      f->received.copy_from(data);
      f->receivedInfo = *info;
      f->dataReceived++;
    };
    int err = dmsg_attach(&ea, &dst, (void*)this, fun, &tag);
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
      checkReceivedValue(expectedValue);
    }
    dataReceived = 0;

    // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
    size_t counter = 0;
    while(dataReceived == 0) {
      // First send, then wait. We assume that after 10 ms the event has been received once the ZMQ mechanism is up and
      // running
      setDoocsValue(expectedValue);
      GlobalFixture::referenceTestApplication.runMainLoopOnce();
      usleep(10000);
      counter++;
      BOOST_REQUIRE(counter < 1000);
    }
    {
      // check first value actually transported via ZMQ protocol
      std::lock_guard<std::mutex> lock(mutex);
      BOOST_CHECK_EQUAL(received.error(), 0);
      checkReceivedValue(expectedValue);
    }

    // make sure no more data is "in the pipeline"
    while(dataReceived > 0) {
      dataReceived = 0;
      usleep(10000);
    }
  }
  ExpectedValueType generateValue(size_t seed) {
    ExpectedValueType v;
    if constexpr(std::is_same_v<ExpectedValueType, int32_t>) {
      v = 42 + seed;
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<int32_t>>) {
      v = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
      v[1] = 100 + seed;
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<float>>) {
      v = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
      v[1] = 100 + seed;
    }
    else {
      BOOST_REQUIRE(false);
    }
    return v;
  }

  void setDoocsValue(ExpectedValueType& expectedValue) {
    if constexpr(std::is_same_v<ExpectedValueType, int32_t>) {
      DoocsServerTestHelper::doocsSet<int32_t>(toDevicePath, expectedValue);
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<int32_t>>) {
      DoocsServerTestHelper::doocsSet<int32_t>(toDevicePath, expectedValue);
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<float>>) {
      DoocsServerTestHelper::doocsSet<float>(toDevicePath, expectedValue);
    }
    else {
      BOOST_REQUIRE(false);
    }
  }
  void checkReceivedValue(ExpectedValueType& expectedValue) {
    if constexpr(std::is_same_v<ExpectedValueType, int32_t>) {
      BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<int32_t>>) {
      BOOST_REQUIRE(received.length() == 10);
      for(size_t k = 0; k < 10; ++k) BOOST_CHECK_EQUAL(received.get_int(k), expectedValue[k]);
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<float>>) {
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_start, 123., 0.001);
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_inc, 0.56, 0.001);
      BOOST_REQUIRE(received.length() == 10);
      for(size_t k = 0; k < 10; ++k) BOOST_CHECK_CLOSE(received.get_float(k), expectedValue[k], 0.0001);
    }
    else {
      BOOST_REQUIRE(false);
    }
  }
  void checkTransport() {
    auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();
    ExpectedValueType expectedValue;
    // From now on, each consistent update should be received.
    // Make sure consistent receiving is happening whether the macro pulse number is send first or second.
    std::array<bool, 10> sendMacroPulseFirst = {true, true, true, false, false, false, true, false, true, false};
    std::array<bool, 10> sendError = {false, false, false, false, false, false, false, false, true, true};
    for(size_t i = 0; i < 10; ++i) {
      GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
      ++macroPulseNumber;
      expectedValue = generateValue(i);
      if(sendMacroPulseFirst[i]) {
        DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_SCALAR", macroPulseNumber);
      }
      else { // send the value first
        setDoocsValue(expectedValue);
      }

      // nothing must be received, no consistent set yet
      BOOST_CHECK_EQUAL(dataReceived, 0);
      GlobalFixture::referenceTestApplication.runMainLoopOnce();
      usleep(10000);
      BOOST_CHECK_EQUAL(dataReceived, 0);

      // now send the variable which has not been send yet
      if(sendMacroPulseFirst[i]) {
        setDoocsValue(expectedValue);
      }
      else {
        DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_SCALAR", macroPulseNumber);
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
        checkReceivedValue(expectedValue);
        // get rid of "/F/D/" prefix
        std::string fromDevicePathTail = fromDevicePath.substr(5);
        VersionNumber v;
        // we know ExpectedValueType is either array or arithmetic base type
        if constexpr(std::is_arithmetic_v<ExpectedValueType>) {
          v = appPVmanager->getProcessArray<ExpectedValueType>(fromDevicePathTail)->getVersionNumber();
        }
        else {
          v = appPVmanager->getProcessArray<typename ExpectedValueType::value_type>(fromDevicePathTail)
                  ->getVersionNumber();
        }
        auto time = v.getTime();
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
        auto usecs =
            std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() - secs * 1e6;
        BOOST_CHECK_EQUAL(receivedInfo.sec, secs);
        BOOST_CHECK_EQUAL(receivedInfo.usec, usecs);
        BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
        BOOST_CHECK_EQUAL(receivedInfo.stat != 0, isError);
        // note, the "status" field in spectrum metadata (of ZMQ payload) is not set as an error field and hence not checked here
      }
    }
  }
  ~ZMQFixture() {
    dmsg_detach(&ea, tag);
    GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::ok;
  }
};

/**********************************************************************************************************************/

BOOST_FIXTURE_TEST_CASE(testScalar, ZMQFixture<int32_t>) {
  std::cout << "testScalar " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  macroPulseNumber = 12345;
  toDevicePath = "//INT/TO_DEVICE_SCALAR";
  fromDevicePath = "/F/D/INT/FROM_DEVICE_SCALAR";
  init();

  checkTransport();
}

/**********************************************************************************************************************/

BOOST_FIXTURE_TEST_CASE(testArray, ZMQFixture<std::vector<int32_t>>) {
  std::cout << "testArray " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  macroPulseNumber = 99999;
  toDevicePath = "//INT/TO_DEVICE_ARRAY";
  fromDevicePath = "/F/D/INT/FROM_DEVICE_ARRAY";

  init();
  checkTransport();
}

/**********************************************************************************************************************/

BOOST_FIXTURE_TEST_CASE(testSpectrum, ZMQFixture<std::vector<float>>) {
  std::cout << "testSpectrum " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  macroPulseNumber = 100;
  toDevicePath = "//FLOAT/TO_DEVICE_ARRAY";
  fromDevicePath = "/F/D/FLOAT/FROM_DEVICE_ARRAY";

  init();
  checkTransport();
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
