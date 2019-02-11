#define BOOST_TEST_MODULE servertestZeroMQ
#include <boost/test/included/unit_test.hpp>

#include <eq_client.h>
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>

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
    ea.adr("doocs://localhost:" + rpc_no + "/F/D/INT/FROM_DEVICE_SCALAR");
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
static std::mutex mutex;
static int expectedValue;
static std::vector<int> expectedArrayValue;
static std::vector<float> expectedFloatArrayValue;

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testScalar) {
  std::cout << "testScalar" << std::endl;
  DoocsLauncher::launchIfNotYetLaunched();

  {
    std::lock_guard<std::mutex> lock(mutex);
    expectedValue = 42;
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", expectedValue);
  }

  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/INT/FROM_DEVICE_SCALAR");
  dmsg_t tag;
  int err = dmsg_attach(&ea,
      &dst,
      nullptr,
      [](void*, EqData* data, dmsg_info_t*) {
        std::lock_guard<std::mutex> lock(mutex);
        BOOST_CHECK_EQUAL(data->error(), 0);
        BOOST_CHECK_EQUAL(data->get_int(), expectedValue);
        dataReceived = true;
      },
      &tag);
  BOOST_CHECK(!err);

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  while(!dataReceived) {
    usleep(1000);
    referenceTestApplication.runMainLoopOnce();
    if(++counter > 10000) break;
  }
  BOOST_CHECK(dataReceived == true);
  usleep(100000); // this sleep is bad, but there seems not to be any better solution - we need to be sure there is no
                  // further pending update
  dataReceived = false;

  // From now on, each update should be received.
  for(size_t i = 0; i < 10; ++i) {
    dataReceived = false;
    usleep(10000);
    BOOST_CHECK(dataReceived == false);
    referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived == true);
    {
      std::lock_guard<std::mutex> lock(mutex);
      expectedValue = 100 + i;
      DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", expectedValue);
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testArray) {
  std::cout << "testArray" << std::endl;
  DoocsLauncher::launchIfNotYetLaunched();

  {
    std::lock_guard<std::mutex> lock(mutex);
    expectedArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_ARRAY", expectedArrayValue);
  }

  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/INT/FROM_DEVICE_ARRAY");
  dmsg_t tag;
  int err = dmsg_attach(&ea,
      &dst,
      nullptr,
      [](void*, EqData* data, dmsg_info_t*) {
        std::lock_guard<std::mutex> lock(mutex);
        BOOST_CHECK_EQUAL(data->error(), 0);
        BOOST_CHECK_EQUAL(data->length(), 10);
        for(size_t i = 0; i < 10; ++i) BOOST_CHECK_EQUAL(data->get_int(i), expectedArrayValue[i]);
        dataReceived = true;
      },
      &tag);
  BOOST_CHECK(!err);

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  while(!dataReceived) {
    usleep(1000);
    referenceTestApplication.runMainLoopOnce();
    if(++counter > 10000) break;
  }
  BOOST_CHECK(dataReceived == true);
  usleep(100000); // this sleep is bad, but there seems not to be any better solution - we need to be sure there is no
                  // further pending update
  dataReceived = false;

  // From now on, each update should be received.
  for(size_t i = 0; i < 10; ++i) {
    dataReceived = false;
    usleep(10000);
    BOOST_CHECK(dataReceived == false);
    referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived == true);
    {
      expectedArrayValue[1] = 100 + i;
      DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_ARRAY", expectedArrayValue);
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testSpectrum) {
  std::cout << "testSpectrum" << std::endl;
  DoocsLauncher::launchIfNotYetLaunched();

  {
    std::lock_guard<std::mutex> lock(mutex);
    expectedFloatArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
    DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
  }

  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/FLOAT/FROM_DEVICE_ARRAY");
  dmsg_t tag;
  int err = dmsg_attach(&ea,
      &dst,
      nullptr,
      [](void*, EqData* data, dmsg_info_t*) {
        std::lock_guard<std::mutex> lock(mutex);
        BOOST_CHECK_EQUAL(data->error(), 0);
        BOOST_CHECK_EQUAL(data->length(), 10);
        BOOST_CHECK_CLOSE(data->get_spectrum()->s_start, 123., 0.001);
        BOOST_CHECK_CLOSE(data->get_spectrum()->s_inc, 0.56, 0.001);
        for(size_t i = 0; i < 10; ++i) BOOST_CHECK_CLOSE(data->get_float(i), expectedFloatArrayValue[i], 0.0001);
        dataReceived = true;
      },
      &tag);
  BOOST_CHECK(!err);

  // The ZeroMQ system in DOOCS is setup in the background, hence we have to try in a loop until we receive the data.
  size_t counter = 0;
  while(!dataReceived) {
    usleep(1000);
    referenceTestApplication.runMainLoopOnce();
    if(++counter > 10000) break;
  }
  BOOST_CHECK(dataReceived == true);
  usleep(100000); // this sleep is bad, but there seems not to be any better solution - we need to be sure there is no
                  // further pending update
  dataReceived = false;

  // From now on, each update should be received.
  for(size_t i = 0; i < 10; ++i) {
    dataReceived = false;
    usleep(10000);
    BOOST_CHECK(dataReceived == false);
    referenceTestApplication.runMainLoopOnce();
    CHECK_WITH_TIMEOUT(dataReceived == true);
    {
      expectedFloatArrayValue[1] = 100 + i;
      DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
    }
  }

  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/
