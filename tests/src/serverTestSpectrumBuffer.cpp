#define BOOST_TEST_MODULE serverTestSpectrumBuffer
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

struct GlobalFixture {
  GlobalFixture() {
    rpcNo = server.rpcNo();
    // wait until server has started (both the update thread and the rpc thread)
    EqCall eq;
    EqAdr ea;
    EqData src, dst;
    ea.adr("doocs://localhost:" + server.rpcNo() + "/F/D/INT/TO_DEVICE_SCALAR");
    std::cout << "doocs://localhost:" + server.rpcNo() + "/F/D/INT/TO_DEVICE_SCALAR" << std::endl;
    while(eq.get(&ea, &src, &dst)) usleep(100000);
    referenceTestApplication.initialiseManualLoopControl();
  }

  ~GlobalFixture() { referenceTestApplication.releaseManualLoopControl(); }

  static ReferenceTestApplication referenceTestApplication;
  static std::string rpcNo;
  ThreadedDoocsServer server{BOOST_STRINGIZE(BOOST_TEST_MODULE) ".conf", framework::master_test_suite().argc,
      framework::master_test_suite().argv};
};
ReferenceTestApplication GlobalFixture::referenceTestApplication{BOOST_STRINGIZE(BOOST_TEST_MODULE)};
std::string GlobalFixture::rpcNo;

BOOST_GLOBAL_FIXTURE(GlobalFixture);

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
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
  }

  // read out the 10 updates from the buffers
  for(size_t i = 0; i < 10; ++i) {
    EqData src, dst;
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
    BOOST_CHECK_EQUAL(rc, 0);
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
    GlobalFixture::referenceTestApplication.runMainLoopOnce();
  }

  // read out the 32 updates from the buffers
  for(size_t i = 0; i < 32; ++i) {
    EqData src, dst;
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
    BOOST_CHECK_EQUAL(rc, 0);
    BOOST_CHECK_EQUAL(dst.error(), 0);
    expectedFloatArrayValue[1] = i + 10000;
    for(size_t k = 0; k < expectedFloatArrayValue.size(); ++k) {
      BOOST_CHECK_CLOSE(dst.get_float(k), expectedFloatArrayValue[k], 1e-6);
    }
  }

  // try requesting macropuse with is just out of range
  {
    EqData src, dst;
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
    BOOST_CHECK_EQUAL(rc, -1);
    BOOST_CHECK_EQUAL(dst.error(), scope_out_of_range);
  }

  // read out the 32 updates from the buffers again
  for(size_t i = 0; i < 32; ++i) {
    EqData src, dst;
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
    BOOST_CHECK_EQUAL(rc, 0);
    BOOST_CHECK_EQUAL(dst.error(), 0);
    expectedFloatArrayValue[1] = i + 10000;
    for(size_t k = 0; k < expectedFloatArrayValue.size(); ++k) {
      BOOST_CHECK_CLOSE(dst.get_float(k), expectedFloatArrayValue[k], 1e-6);
    }
  }
}

/**********************************************************************************************************************/
