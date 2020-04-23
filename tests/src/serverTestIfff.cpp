#define BOOST_TEST_MODULE serverTestXy
#include <boost/test/included/unit_test.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <eq_client.h>
#include <random>
#include <thread>
#include <algorithm>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

static ReferenceTestApplication referenceTestApplication("serverTestIfff");

const std::string PROPERTY_NAME{"//CUSTOM/IFFF"};

/**********************************************************************************************************************/

extern int eq_server(int, char**);

struct DoocsLauncher {
  DoocsLauncher() {
    // choose random RPC number
    std::random_device rd;
    std::uniform_int_distribution<int> dist(620000000, 999999999);
    rpc_no = std::to_string(dist(rd));
    // update config file with the RPC number
    std::string command = "sed -i serverTestIfff.conf -e "
                          "'s/^SVR.RPC_NUMBER:.*$/SVR.RPC_NUMBER: " +
        rpc_no + "/'";
    auto rc = std::system(command.c_str());
    (void)rc;

    // start the server
    doocsServerThread = std::thread(eq_server, boost::unit_test::framework::master_test_suite().argc,
        boost::unit_test::framework::master_test_suite().argv);
    // wait until server has started (both the update thread and the rpc thread)
    EqCall eq;
    EqAdr ea;
    EqData src, dst;
    ea.adr("doocs://localhost:" + rpc_no + "/F/D/INT/TO_DEVICE_SCALAR");
    while(eq.get(&ea, &src, &dst)) usleep(100000);
    referenceTestApplication.initialiseManualLoopControl();
  }

  /**
 * @brief For compatibility with older DOOCS versions declare our own eq_exit
 *
 * Can be removed once a new doocs server version is released.
 */
  void eq_exit() {
    auto nativeHandle = doocsServerThread.native_handle();
    if(nativeHandle != 0) pthread_kill(nativeHandle, SIGTERM);
  }

  ~DoocsLauncher() {
    referenceTestApplication.releaseManualLoopControl();
    eq_exit();
    doocsServerThread.join();
  }

  std::thread doocsServerThread;
  static std::string rpc_no;
};
std::string DoocsLauncher::rpc_no;

BOOST_GLOBAL_FIXTURE(DoocsLauncher);

/**********************************************************************************************************************/
BOOST_AUTO_TEST_CASE(testIfffUpdate) {
  std::cout << "testIfffUpdate" << std::endl;

  auto extractValue = []() -> IFFF {
    auto d_ifff = getDoocsProperty<D_ifff>(PROPERTY_NAME);
    auto location = getLocationFromPropertyAddress(PROPERTY_NAME);
    IFFF value; // a copy of the value. We don't want to hold the location lock longer than needed

    location->lock();
    value = *(
        d_ifff
            ->value()); // value returs a pointer which we only must dereference while holding the lock. So we make a copy
    location->unlock();
    return value;
  };

  auto value = extractValue();
  // FIXME put correct vales what to expect
  BOOST_CHECK_CLOSE(value.f1_data, 123., 0.0001);
}

/**********************************************************************************************************************/
