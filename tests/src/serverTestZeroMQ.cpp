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
  }

  static void launchIfNotYetLaunched() { static DoocsLauncher launcher; }

  static std::string rpc_no;
};
std::string DoocsLauncher::rpc_no;

/**********************************************************************************************************************/

static std::atomic<bool> dataReceived;
static std::atomic<int> expectedValue;

BOOST_AUTO_TEST_CASE(testZeroMQ) {
  DoocsLauncher::launchIfNotYetLaunched();
  referenceTestApplication.initialiseManualLoopControl();
  dmsg_start();

  expectedValue = 42;
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", expectedValue);

  EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/INT/FROM_DEVICE_SCALAR");
  dmsg_t tag;
  int err = dmsg_attach(&ea,
      &dst,
      nullptr,
      [](void*, EqData* data, dmsg_info_t*) {
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
    expectedValue = 100 + i;
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", expectedValue);
  }

  referenceTestApplication.releaseManualLoopControl();
  dmsg_detach(&ea, tag);
}

/**********************************************************************************************************************/
