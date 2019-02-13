#define BOOST_TEST_MODULE servertestZeroMQ
#include <boost/test/included/unit_test.hpp>

#include <thread>
#include <random>
#include <eq_client.h>
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include "serverBasedTestTools.h"
#include "DoocsAdapter.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

static ReferenceTestApplication referenceTestApplication("serverTestZeroMQ");

/**********************************************************************************************************************/

extern int eq_server(int, char**);

struct DoocsLauncher {
  DoocsLauncher() {
    // choose random RPC number
    std::random_device rd;
    std::uniform_int_distribution<int> dist(620000000, 999999999);
    rpc_no = std::to_string(dist(rd));
    // update config file with the RPC number
    std::string command = "sed -i serverTestZeroMQ.conf -e 's/^SVR.RPC_NUMBER:.*$/SVR.RPC_NUMBER: " + rpc_no + "/'";
    auto rc = std::system(command.c_str());
    (void)rc;

    // start the server
    std::thread(eq_server,
        boost::unit_test::framework::master_test_suite().argc,
        boost::unit_test::framework::master_test_suite().argv)
        .detach();
    // wait until server has started (both the update thread and the rpc thread)
    DoocsServerTestHelper::runUpdate();
    EqCall eq;
    EqAdr ea;
    EqData src, dst;
    ea.adr("doocs://localhost:" + rpc_no + "/F/D/UINT/FROM_DEVICE_SCALAR");
    while(eq.get(&ea, &src, &dst)) usleep(100000);
    dmsg_start();
    referenceTestApplication.initialiseManualLoopControl();
  }
  ~DoocsLauncher() { referenceTestApplication.releaseManualLoopControl(); }

  static void launchIfNotYetLaunched() { static DoocsLauncher launcher; }

  static std::string rpc_no;
};
std::string DoocsLauncher::rpc_no;

/**********************************************************************************************************************/

static std::atomic<bool> dataReceived;
static EqData received;
static dmsg_info_t receivedInfo;
static std::mutex mutex;

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testScalar) {
  std::cout << "testScalar" << std::endl;
  DoocsLauncher::launchIfNotYetLaunched();

  auto appPVmanager = referenceTestApplication.getPVManager();

  /// Note: The data is processed by the ReferenceTestApplication in the order of the types as listed in the HolderMap
  /// of the ReferenceTestApplication. INT comes before UINT and FLOAT, so the macro pulse number is first written and
  /// then our values.

  int macroPulseNumber = 12345;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);

  uint32_t expectedValue = 42;
  DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);

  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/UINT/FROM_DEVICE_SCALAR");
  dmsg_t tag;
  int err = dmsg_attach(&ea,
      &dst,
      nullptr,
      [](void*, EqData* data, dmsg_info_t* info) {
        std::lock_guard<std::mutex> lock(mutex);
        received.copy_from(data);
        receivedInfo = *info;
        dataReceived = true;
      },
      &tag);
  BOOST_CHECK(!err);

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  dataReceived = false;
  while(!dataReceived) {
    usleep(1000);
    referenceTestApplication.runMainLoopOnce();
    if(++counter > 10000) break;
  }
  BOOST_CHECK(dataReceived == true);
  {
    std::lock_guard<std::mutex> lock(mutex);
    BOOST_CHECK_EQUAL(received.error(), 0);
    BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
  }
  usleep(100000); // this sleep is bad, but there seems not to be any better solution - we need to be sure there is no
                  // further pending update

  // From now on, each update should be received.
  for(size_t i = 0; i < 10; ++i) {
    dataReceived = false;
    usleep(10000);
    BOOST_CHECK(dataReceived == false);
    referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived == true);
    {
      std::lock_guard<std::mutex> lock(mutex);
      BOOST_CHECK_EQUAL(received.error(), 0);
      BOOST_CHECK_EQUAL(received.get_int(), expectedValue);
      auto time = appPVmanager->getProcessArray<uint32_t>("UINT/FROM_DEVICE_SCALAR")->getVersionNumber().getTime();
      auto secs = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
      auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() - secs * 1e6;
      BOOST_CHECK_EQUAL(receivedInfo.sec, secs);
      BOOST_CHECK_EQUAL(receivedInfo.usec, usecs);
      BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
      ++macroPulseNumber;
      DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
      expectedValue = 100 + i;
      DoocsServerTestHelper::doocsSet<uint32_t>("//UINT/TO_DEVICE_SCALAR", expectedValue);
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testArray) {
  std::cout << "testArray" << std::endl;
  DoocsLauncher::launchIfNotYetLaunched();

  auto appPVmanager = referenceTestApplication.getPVManager();

  int macroPulseNumber = 99999;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);

  std::vector<int32_t> expectedArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
  DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", expectedArrayValue);

  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/UINT/FROM_DEVICE_ARRAY");
  dmsg_t tag;
  int err = dmsg_attach(&ea,
      &dst,
      nullptr,
      [](void*, EqData* data, dmsg_info_t* info) {
        std::lock_guard<std::mutex> lock(mutex);
        received.copy_from(data);
        receivedInfo = *info;
        dataReceived = true;
      },
      &tag);
  BOOST_CHECK(!err);

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  dataReceived = false;
  while(!dataReceived) {
    usleep(1000);
    referenceTestApplication.runMainLoopOnce();
    if(++counter > 10000) break;
  }
  BOOST_CHECK(dataReceived == true);
  {
    std::lock_guard<std::mutex> lock(mutex);
    BOOST_CHECK_EQUAL(received.error(), 0);
    BOOST_CHECK_EQUAL(received.length(), 10);
    for(size_t k = 0; k < 10; ++k) BOOST_CHECK_EQUAL(received.get_int(k), expectedArrayValue[k]);
  }
  usleep(100000); // this sleep is bad, but there seems not to be any better solution - we need to be sure there is no
                  // further pending update

  // From now on, each update should be received.
  for(size_t i = 0; i < 10; ++i) {
    dataReceived = false;
    usleep(10000);
    BOOST_CHECK(dataReceived == false);
    referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived == true);
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
      --macroPulseNumber;
      DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
      expectedArrayValue[1] = 100 + i;
      DoocsServerTestHelper::doocsSet<int32_t>("//UINT/TO_DEVICE_ARRAY", expectedArrayValue);
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testSpectrum) {
  std::cout << "testSpectrum" << std::endl;
  DoocsLauncher::launchIfNotYetLaunched();

  auto appPVmanager = referenceTestApplication.getPVManager();

  int macroPulseNumber = -100;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);

  std::vector<float> expectedFloatArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);

  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/FLOAT/FROM_DEVICE_ARRAY");
  dmsg_t tag;
  int err = dmsg_attach(&ea,
      &dst,
      nullptr,
      [](void*, EqData* data, dmsg_info_t* info) {
        std::lock_guard<std::mutex> lock(mutex);
        received.copy_from(data);
        receivedInfo = *info;
        dataReceived = true;
      },
      &tag);
  BOOST_CHECK(!err);

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  dataReceived = false;
  while(!dataReceived) {
    usleep(1000);
    referenceTestApplication.runMainLoopOnce();
    if(++counter > 10000) break;
  }
  BOOST_CHECK(dataReceived == true);
  BOOST_CHECK_EQUAL(received.error(), 0);
  BOOST_CHECK_EQUAL(received.length(), 10);
  BOOST_CHECK_CLOSE(received.get_spectrum()->s_start, 123., 0.001);
  BOOST_CHECK_CLOSE(received.get_spectrum()->s_inc, 0.56, 0.001);
  for(size_t k = 0; k < 10; ++k) BOOST_CHECK_CLOSE(received.get_float(k), expectedFloatArrayValue[k], 0.0001);
  usleep(100000); // this sleep is bad, but there seems not to be any better solution - we need to be sure there is no
                  // further pending update

  // From now on, each update should be received.
  for(size_t i = 0; i < 10; ++i) {
    dataReceived = false;
    usleep(10000);
    BOOST_CHECK(dataReceived == false);
    referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived == true);
    {
      std::lock_guard<std::mutex> lock(mutex);
      BOOST_CHECK_EQUAL(received.error(), 0);
      BOOST_CHECK_EQUAL(received.length(), 10);
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_start, 123., 0.001);
      BOOST_CHECK_CLOSE(received.get_spectrum()->s_inc, 0.56, 0.001);
      for(size_t k = 0; k < 10; ++k) BOOST_CHECK_CLOSE(received.get_float(k), expectedFloatArrayValue[k], 0.0001);
      auto time = appPVmanager->getProcessArray<float>("FLOAT/FROM_DEVICE_ARRAY")->getVersionNumber().getTime();
      auto secs = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
      auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() - secs * 1e6;
      BOOST_CHECK_EQUAL(receivedInfo.sec, secs);
      BOOST_CHECK_EQUAL(receivedInfo.usec, usecs);
      BOOST_CHECK_EQUAL(receivedInfo.ident, macroPulseNumber);
      macroPulseNumber *= -2;
      DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", macroPulseNumber);
      expectedFloatArrayValue[1] = 100 + i;
      DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/
