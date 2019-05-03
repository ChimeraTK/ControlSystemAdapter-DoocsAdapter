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

static ReferenceTestApplication referenceTestApplication("serverTestXy");

const std::string PROPERTY_NAME{"//FLOAT/TEST_XY"};

/**********************************************************************************************************************/

extern int eq_server(int, char**);

struct DoocsLauncher {
  DoocsLauncher() {
    // choose random RPC number
    std::random_device rd;
    std::uniform_int_distribution<int> dist(620000000, 999999999);
    rpc_no = std::to_string(dist(rd));
    // update config file with the RPC number
    std::string command = "sed -i serverTestXy.conf -e "
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

BOOST_AUTO_TEST_CASE(testXyMetadata) {
  std::cout << "testXyMetadata" << std::endl;

  auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
  BOOST_CHECK_EQUAL(xy->description(), "Some XY test");
  BOOST_CHECK_EQUAL(xy->max_length(), 10);
  float f1, f2;
  int i1;
  time_t tmp;
  char buf[255];

  xy->plot_x_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
  BOOST_CHECK_EQUAL(buf, "X_DESCRIPTION");
  xy->plot_y_value(&i1, &f1, &f2, &tmp, buf, sizeof(buf));
  BOOST_CHECK_EQUAL(buf, "Y_DESCRIPTION");
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testXyUpdates) {
  std::cout << "testXy" << std::endl;

  auto check = []() -> double {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    auto location = getLocationFromPropertyAddress(PROPERTY_NAME);
    double value;

    location->lock();
    value = xy->xy()->y_data;
    location->unlock();

    return value;
  };

  // Send values with inconsistent versions
  std::vector<float> xArrayValues = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", xArrayValues);
  std::vector<double> yArrayValues = {-10.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0f};
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);

  referenceTestApplication.runMainLoopOnce();

  // Wait a bit
  usleep(2000000);

  // Nothing should have happened
  {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, 0.0f, 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, 0.0f, 1e-6f);
    }
  }

  referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", xArrayValues);
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  referenceTestApplication.runMainLoopOnce();

  checkWithTimeout<double>(check, -10.0);

  // Nothing should have happened
  {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i], 1e-6f);
    }
  }

  /* Just updating Y values with matching version */
  std::transform(yArrayValues.begin(), yArrayValues.end(), yArrayValues.begin(), [](double v) { return v * 10.0; });
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  referenceTestApplication.runMainLoopOnce();

  checkWithTimeout<double>(check, -10.0 * 10.0);
  // Nothing should have happened
  {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i], 1e-6f);
    }
  }

  std::transform(yArrayValues.begin(), yArrayValues.end(), yArrayValues.begin(), [](double v) { return v * 10.0; });

  referenceTestApplication.versionNumber = boost::none;
  DoocsServerTestHelper::doocsSet<double>("//DOUBLE/TO_DEVICE_ARRAY", yArrayValues);
  referenceTestApplication.runMainLoopOnce();

  // Wait a bit
  usleep(2000000);

  // Nothing should have happened
  {
    auto xy = getDoocsProperty<D_xy>(PROPERTY_NAME);
    for(int i = 0; i < xy->max_length(); i++) {
      auto val = xy->value(i);
      BOOST_CHECK_CLOSE(val->x_data, xArrayValues[i], 1e-6f);
      BOOST_CHECK_CLOSE(val->y_data, yArrayValues[i] / 10.0, 1e-6f);
    }
  }

#if 0
  EqAdr ea;
  ea.adr("doocs://localhost:" + DoocsLauncher::rpc_no + "/F/D/FLOAT/FROM_DEVICE_ARRAY");

  auto appPVmanager = referenceTestApplication.getPVManager();

  int firstMacroPulseNumber = 648583; // just some random number to start with

  // starting value for the array
  std::vector<float> expectedFloatArrayValue = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

  // fill 10 updates (we have 32 buffers)
  for(size_t i = 0; i < 10; ++i) {
    DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", firstMacroPulseNumber + i);
    expectedFloatArrayValue[1] = i;
    DoocsServerTestHelper::doocsSet<float>("//FLOAT/TO_DEVICE_ARRAY", expectedFloatArrayValue);
    referenceTestApplication.runMainLoopOnce();
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
    referenceTestApplication.runMainLoopOnce();
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
#endif
}

/**********************************************************************************************************************/
