#define BOOST_TEST_MODULE serverTestDataMatching
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
//#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <eq_client.h>

#include <vector>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

//DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION
static ReferenceTestApplication referenceTestApplication("serverTestDataMatching");

extern int eq_server(int, char**);

struct DoocsLauncher {
  DoocsLauncher() {
    // choose random RPC number
    std::random_device rd;
    std::uniform_int_distribution<int> dist(620000000, 999999999);
    rpc_no = std::to_string(dist(rd));
    // update config file with the RPC number
    std::string command = "sed -i serverTestDataMatching.conf -e "
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
    ea.adr("doocs://localhost:" + rpc_no + "/F/D/UINT/FROM_DEVICE_SCALAR");
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

BOOST_GLOBAL_FIXTURE(DoocsLauncher)

/**********************************************************************************************************************/

/// Note: The data is processed by the ReferenceTestApplication in the order
/// of the types as listed in the HolderMap of the ReferenceTestApplication.
/// INT comes before UINT and FLOAT, so the macro pulse number is first
/// written and then our values.

BOOST_AUTO_TEST_CASE(testProcessScalar) {
  std::cout << "testProcessScalar" << std::endl;

  auto appPVmanager = referenceTestApplication.getPVManager();

  // We need data consistency between macro pulse number and the data
  // Everything from now on will get the same version number, until we manually
  // set a new one.
  referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

  int macroPulseNumber = 12345;
  DoocsServerTestHelper::doocsSet<int>("//UNMAPPED/INT.TO_DEVICE_SCALAR", macroPulseNumber);

  unsigned int expectedUnsignedInt = 42U;
  float expectedFloat = 42.42f;
  DoocsServerTestHelper::doocsSet<unsigned int>("//UINT/TO_DEVICE_SCALAR", expectedUnsignedInt);
  DoocsServerTestHelper::doocsSet<float>("//UNMAPPED/FLOAT.TO_DEVICE_SCALAR", expectedFloat);

  referenceTestApplication.runMainLoopOnce();

  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<unsigned int>("//UINT/FROM_DEVICE_SCALAR") - expectedUnsignedInt < 1e-6);
  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<float>("//UNMAPPED/FLOAT.FROM_DEVICE_SCALAR") - expectedFloat < 1e-6);

  // Send MPN and values with different version numbers
  referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  referenceTestApplication.runMainLoopOnce();

  referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  auto lastExpectedUnsignedInt = expectedUnsignedInt;
  expectedUnsignedInt = 2U;
  expectedFloat = 2.42f;
  DoocsServerTestHelper::doocsSet<unsigned int>("//UINT/TO_DEVICE_SCALAR", expectedUnsignedInt);
  DoocsServerTestHelper::doocsSet<float>("//UNMAPPED/FLOAT.TO_DEVICE_SCALAR", expectedFloat);

  referenceTestApplication.runMainLoopOnce();

  // Unsigned int value must not be updated because it has data matching exact
  // FIXME Better way to check this?
  usleep(1000000);
  BOOST_CHECK_EQUAL(
      DoocsServerTestHelper::doocsGet<unsigned int>("//UINT/FROM_DEVICE_SCALAR"), lastExpectedUnsignedInt);

  // Float value has to get the update even with version number mismatch
  // because data matching is none
  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<float>("//UNMAPPED/FLOAT.FROM_DEVICE_SCALAR") - expectedFloat < 1e-6);

  // Another iteration, now back to consistent version number
  referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  ++macroPulseNumber;
  DoocsServerTestHelper::doocsSet<int>("//UNMAPPED/INT.TO_DEVICE_SCALAR", macroPulseNumber);
  expectedFloat = 12.42f;
  DoocsServerTestHelper::doocsSet<float>("//UNMAPPED/FLOAT.TO_DEVICE_SCALAR", expectedFloat);
  referenceTestApplication.runMainLoopOnce();
  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<float>("//UNMAPPED/FLOAT.FROM_DEVICE_SCALAR") - expectedFloat < 1e-6);
}

/**********************************************************************************************************************/
