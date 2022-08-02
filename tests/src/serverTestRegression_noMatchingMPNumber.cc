// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestRegression_noMatchingMPNumber
#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <boost/test/included/unit_test.hpp>
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

  // sleep(10);
  //  set initial values
  int macroPulseNumber = 12345;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == macroPulseNumber);

  // now the new value must propagate with the correct MPN, via RPC or ZMQ does not matter
  uint32_t expectedValue = 42;
  DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);

  /// Note: The data is processed by the ReferenceTestApplication in the order
  /// of the types as listed in the HolderMap of the ReferenceTestApplication.
  /// INT comes before UINT and FLOAT, so the macro pulse number is first
  /// written and then our values.
  size_t retryCounter = 0;
retry:
  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/UINT/TO_DEVICE_SCALAR");
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
    // First send, then wait. We assume that after 10 ms the event has been received once the ZMQ mechanism is up and
    // running
    DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);
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
    BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
    BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
  }

  while(dataReceived == 0) {
    // First send, then wait. We assume that after 10 ms the event has been received once the ZMQ mechanism is up and
    // running
    DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);
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
    BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
    BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
  }

  dmsg_detach(&ea, tag);
}

BOOST_AUTO_TEST_CASE(testArray) {
  std::cout << "testArray " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();

  // We need data consistency between macro pulse number and the data
  // Everything from now on will get the same version number, until we manually set a new one.
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

  // set initial values
  int macroPulseNumber = 12346;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == macroPulseNumber);

  // now the new value must propagate with the correct MPN, via RPC or ZMQ does not matter

  int expectedValue = 42;
  DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", std::initializer_list<int>{expectedValue});

  /// Note: The data is processed by the ReferenceTestApplication in the order
  /// of the types as listed in the HolderMap of the ReferenceTestApplication.
  /// INT comes before UINT and FLOAT, so the macro pulse number is first
  /// written and then our values.
  size_t retryCounter = 0;
retry:
  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/UINT/TO_DEVICE_ARRAY");
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
    // First send, then wait. We assume that after 10 ms the event has been received once the ZMQ mechanism is up and
    // running
    DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", std::initializer_list<int>{expectedValue});
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
    BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
    BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
  }

  dmsg_detach(&ea, tag);
}

BOOST_AUTO_TEST_CASE(testIff) {
  std::cout << "testIFFF" << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();

  // We need data consistency between macro pulse number and the data
  // Everything from now on will get the same version number, until we manually set a new one.
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

  // set initial values
  int macroPulseNumber = 12347;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == macroPulseNumber);

  // now the new value must propagate with the correct MPN, via RPC or ZMQ does not matter

  IFFF ifff;
  ifff.f1_data = 1.0;
  ifff.f2_data = 1.0;
  ifff.f3_data = 1.0;
  ifff.i1_data = 1;

  int expectedValue = 0;
  DoocsServerTestHelper::doocsSet<IFFF>("//CUSTOM/IFFF", ifff);

  /// Note: The data is processed by the ReferenceTestApplication in the order
  /// of the types as listed in the HolderMap of the ReferenceTestApplication.
  /// INT comes before UINT and FLOAT, so the macro pulse number is first
  /// written and then our values.
  size_t retryCounter = 0;
retry:
  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/CUSTOM/IFFF");
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
    // First send, then wait. We assume that after 10 ms the event has been received once the ZMQ mechanism is up and
    // running
    DoocsServerTestHelper::doocsSet<IFFF>("//CUSTOM/IFFF", ifff);
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
    BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
    BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
  }

  dmsg_detach(&ea, tag);
}
