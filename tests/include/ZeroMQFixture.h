// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

#include <boost/test/unit_test_suite.hpp>

#include <eq_dmsg.h>
#include <eq_fct.h>

#include <cstdint>

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION_WITH_CODE(dmsg_start();)

template<typename SetValueType, typename ExpectedValueType = SetValueType>
struct ZeroMQFixture {
  std::atomic<uint32_t> dataReceived = 0;
  doocs::EqData received;
  dmsg_info_t receivedInfo{};
  std::mutex mutex;

  doocs::EqData dst;
  EqAdr ea;

  dmsg_t tag{};

  // test input config parameters
  // doocs address for writing SetValueType
  std::string toDevicePath;
  // doocs address for reading ExpectedValueType
  std::string fromDevicePath;
  // application pv path for expected meta data (time stamp); optional
  std::string fromDeviceMetaDataPath;
  // starting value for macro pulse number
  uint32_t macroPulseNumber = 1;
  // should checkTransport put Application to invalid state and check received err
  bool generateErrors = true;
  // should checkTransport() assume that updates must be consistent
  bool configUsesDataMatching = true;

  void init() {
    // We need data consistency between macro pulse number and the data
    // Everything from now on will get the same version number, until we manually set a new one.
    GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

    // set initial values
    // we use the UINT variable for the macropulse number because we want INT to be reserved for actual values
    DoocsServerTestHelper::doocsSet<int>("//UINT/TO_DEVICE_SCALAR", macroPulseNumber);

    // this makes sure initial value that we read back should be valid
    SetValueType setValue = generateValue<SetValueType>(0);
    ExpectedValueType expectedValue = generateValue<ExpectedValueType>(0);
    setDoocsValue(setValue);
    GlobalFixture::referenceTestApplication.runMainLoopOnce();

    // Note: The data is processed by the ReferenceTestApplication in the order
    // of the types as listed in the HolderMap of the ReferenceTestApplication.
    // build address of the form  doocs://localhost:628635561/F/D/location/property
    ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D" + fromDevicePath.substr(1));
    dataReceived = 0;
    auto fun = [](void* fixture, doocs::EqData* data, dmsg_info_t* info) {
      auto* f = static_cast<ZeroMQFixture*>(fixture);
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
      setDoocsValue(setValue);
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
  template<typename ValueType>
  static ValueType generateValue(size_t seed) {
    ValueType v;
    if constexpr(std::is_same_v<ValueType, int32_t>) {
      v = 42 + (int32_t)seed;
    }
    else if constexpr(std::is_same_v<ValueType, std::vector<int32_t>>) {
      v = {42, 43 + (int32_t)seed, 44, 45, 46, 47, 48, 49, 50, 51};
    }
    else if constexpr(std::is_same_v<ValueType, std::vector<float>>) {
      v = {42, 43, 44 + (float)seed, 45, 46, 47, 48, 49, 50, 51};
    }
    else if constexpr(std::is_same_v<ValueType, IFFF>) {
      // we have a test where i1_value is mapped as generated int value!
      v = {42 + (int32_t)seed, (float)(11.0 + seed), 12.0, 13.0};
    }
    else {
      BOOST_REQUIRE(false);
    }
    return v;
  }

  void setDoocsValue(SetValueType& value) {
    if constexpr(std::is_same_v<SetValueType, std::vector<int32_t>>) {
      DoocsServerTestHelper::doocsSet<int32_t>(toDevicePath, value);
    }
    else if constexpr(std::is_same_v<SetValueType, std::vector<float>>) {
      DoocsServerTestHelper::doocsSet<float>(toDevicePath, value);
    }
    else {
      DoocsServerTestHelper::doocsSet<SetValueType>(toDevicePath, value);
    }
  }
  void checkReceivedValue(ExpectedValueType& expectedValue) {
    if constexpr(std::is_same_v<ExpectedValueType, int32_t>) {
      BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<int32_t>>) {
      BOOST_REQUIRE(received.length() == 10);
      for(size_t k = 0; k < 10; ++k) {
        BOOST_CHECK_EQUAL(received.get_int(k), expectedValue[k]);
      }
    }
    else if constexpr(std::is_same_v<ExpectedValueType, std::vector<float>>) {
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_start, 123., 0.001);
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_inc, 0.56, 0.001);
      BOOST_REQUIRE(received.length() == 10);
      for(size_t k = 0; k < 10; ++k) {
        BOOST_CHECK_CLOSE(received.get_float(k), expectedValue[k], 0.0001);
      }
    }
    else if constexpr(std::is_same_v<ExpectedValueType, IFFF>) {
      BOOST_CHECK_EQUAL(received.get_ifff()->i1_data, expectedValue.i1_data);
      BOOST_CHECK_CLOSE(received.get_ifff()->f1_data, expectedValue.f1_data, 0.001);
      BOOST_CHECK_CLOSE(received.get_ifff()->f2_data, expectedValue.f2_data, 0.001);
      BOOST_CHECK_CLOSE(received.get_ifff()->f3_data, expectedValue.f3_data, 0.001);
    }
    else {
      BOOST_REQUIRE(false);
    }
  }
  void checkTransport() {
    auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();
    // From now on, each consistent update should be received.
    // Make sure consistent receiving is happening whether the macro pulse number is send first or second.
    std::array<bool, 10> sendMacroPulseFirst = {true, true, true, false, false, false, true, false, true, false};
    std::array<bool, 10> sendError = {false, false, false, false, false, false, false, false, true, true};
    for(size_t i = 0; i < 10; ++i) {
      bool isErrorCase = sendError[i];
      if(isErrorCase && not generateErrors) {
        continue;
      }

      GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
      ++macroPulseNumber;
      SetValueType setValue = generateValue<SetValueType>(i);
      ExpectedValueType expectedValue = generateValue<ExpectedValueType>(i);
      if(sendMacroPulseFirst[i]) {
        DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_SCALAR", macroPulseNumber);
      }
      else { // send the value first
        setDoocsValue(setValue);
      }

      if(configUsesDataMatching) {
        // nothing must be received, no consistent set yet
        BOOST_CHECK_EQUAL(dataReceived, 0);
        GlobalFixture::referenceTestApplication.runMainLoopOnce();
        usleep(10000);
        BOOST_CHECK_EQUAL(dataReceived, 0);
      }
      else {
        // data_matching = none in test config means we expect not-yet-consistent updates
        GlobalFixture::referenceTestApplication.runMainLoopOnce();
        usleep(10000);
        if(sendMacroPulseFirst[i]) {
          // we don't expect an update to be sent out just for mp update
          BOOST_CHECK_EQUAL(dataReceived, 0);
        }
      }

      // now send the variable which has not been send yet
      if(sendMacroPulseFirst[i]) {
        setDoocsValue(setValue);
      }
      else {
        DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_SCALAR", macroPulseNumber);
      }
      if(isErrorCase) {
        GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::faulty;
      }
      else {
        GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::ok;
      }
      GlobalFixture::referenceTestApplication.runMainLoopOnce();

      CHECK_WITH_TIMEOUT(dataReceived > 0);
      usleep(10000);
      BOOST_CHECK_EQUAL(dataReceived, 1);
      dataReceived = 0;
      {
        std::lock_guard<std::mutex> lock(mutex);
        BOOST_CHECK_EQUAL(received.error() != 0, isErrorCase);
        checkReceivedValue(expectedValue);

        std::string metaDataPVPath = fromDeviceMetaDataPath.length() > 0 ? fromDeviceMetaDataPath : fromDevicePath;
        ChimeraTK::VersionNumber v = appPVmanager->getProcessVariable(metaDataPVPath)->getVersionNumber();
        auto time = v.getTime();
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
        auto usecs =
            std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() - secs * 1e6;
        BOOST_CHECK_EQUAL(receivedInfo.sec, secs);
        BOOST_CHECK_EQUAL(receivedInfo.usec, usecs);
        if(configUsesDataMatching) {
          BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
        }
        BOOST_CHECK_EQUAL(receivedInfo.stat != 0, isErrorCase);
        // note, the "status" field in spectrum metadata (of ZMQ payload) is not set as an error field and hence not checked here
      }
    }
  }
  ~ZeroMQFixture() {
    // TODO - find a way to make this faster, it's the main slowdown of this test
    dmsg_detach(&ea, tag);
    GlobalFixture::referenceTestApplication.dataValidity = ChimeraTK::DataValidity::ok;
  }
};
