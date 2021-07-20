#define BOOST_TEST_MODULE serverTestArrayPersistence
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

#include <vector>
#include <fstream>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

template<typename T>
void vectorToFile(std::vector<T>& vec, const char* fname) {
  std::ofstream of(fname);
  for(auto num : vec) of << num << std::endl;
  of.close();
}
template<typename T>
void vectorFromFile(std::vector<T>& vec, const char* fname) {
  std::ifstream ifs(fname);
  size_t j = 0;
  T num;
  while((ifs >> num) && j < vec.size()) {
    vec[j++] = num;
  }
  ifs.close();
}

const int alen = 40;

// slightly modified GlobalFixture from serverBasedTestTools.h
struct GlobalFixture {
  GlobalFixture() {
    cleanupFiles();
    rpcNo = server.rpcNo();
    bpn = server.bpn();
    ChimeraTK::DoocsAdapter::waitUntilInitialised();
    GlobalFixture::referenceTestApplication.initialiseManualLoopControl();
  }
  void cleanupFiles() {
    boost::filesystem::remove_all("hist");
    boost::filesystem::create_directory("hist");

    std::vector<int> vint(alen);
    std::vector<double> vdouble(alen);
    for(int i = 0; i < alen; i++) {
      vint[i] = 100 + i;
      vdouble[i] = 200.1 + i;
    }
    vectorToFile(vint, "hist/INT-TO_DEVICE_ARRAY.intarray");
    vectorToFile(vdouble, "hist/DOUBLE-TO_DEVICE_ARRAY.doublearray");
  }

  ~GlobalFixture() { GlobalFixture::referenceTestApplication.releaseManualLoopControl(); }

  static ReferenceTestApplication referenceTestApplication;
  static std::string rpcNo;
  static std::string bpn;
  ThreadedDoocsServer server{boost::unit_test::framework::master_test_suite().p_name.value + ".conf",
      boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv};
};

ReferenceTestApplication GlobalFixture::referenceTestApplication{BOOST_STRINGIZE(BOOST_TEST_MODULE), alen};
std::string GlobalFixture::rpcNo;
std::string GlobalFixture::bpn;

CTK_BOOST_GLOBAL_FIXTURE(GlobalFixture)

// the array must have testStartValue+i at index i.
template<class T>
static bool testArrayContent1(std::vector<T>& array, std::string const& propertyName, T testStartValue, T delta) {
  bool isOK = true;
  T currentTestValue = testStartValue;
  size_t index = 0;
  for(auto val : array) {
    if(std::fabs(val - currentTestValue) > 0.001) {
      if(isOK) {
        std::cout << "Array " << propertyName << " does not contain expected values. First mismatching data at index "
                  << index << ": " << val << " != " << currentTestValue << std::endl;
      }
      isOK = false;
    }
    currentTestValue += delta;
    ++index;
  }
  return isOK;
}
template<class T>
static bool testArrayContent(std::string const& propertyName, T testStartValue, T delta) {
  auto array = DoocsServerTestHelper::doocsGetArray<T>(propertyName);
  return testArrayContent1(array, propertyName, testStartValue, delta);
}

/// Using prepared persistence file, start server, check restored values, change values, trigger persisting, compare file with expectation
BOOST_AUTO_TEST_CASE(testArrayWrite) {
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  std::string const& propertyAddress = "//INT/MY_RENAMED_INTARRAY";
  checkDataType(propertyAddress, DATA_A_INT);

  CHECK_WITH_TIMEOUT(testArrayContent<float>("//INT/MY_RENAMED_INTARRAY", 100, 1) == true);
  CHECK_WITH_TIMEOUT(testArrayContent<float>("//DOUBLE/FROM_DEVICE_ARRAY", 200.1, 1) == true);

  std::vector<int> vint(alen);
  std::vector<double> vdouble(alen);
  for(int i = 0; i < alen; i++) {
    vint[i] = 150 + i;
    vdouble[i] = 250.3 + i;
  }
  DoocsServerTestHelper::doocsSet("//INT/TO_DEVICE_ARRAY", vint);
  DoocsServerTestHelper::doocsSet("//DOUBLE/TO_DEVICE_ARRAY", vdouble);

  // run the application loop.
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // check changed values
  usleep(100000);
  CHECK_WITH_TIMEOUT(testArrayContent<float>("//INT/MY_RENAMED_INTARRAY", 150, 1) == true);
  CHECK_WITH_TIMEOUT(testArrayContent<float>("//DOUBLE/FROM_DEVICE_ARRAY", 250.3, 1) == true);

  // trigger array storing
  DoocsServerTestHelper::doocsSet<bool>("//ARRAY_PERSISTENCE_TEST._SVR/SVR.SAVE", true);
  // wait till disk file written
  usleep(500000);

  // check persisted file against known values
  std::vector<int> vint2(alen);
  std::vector<double> vdouble2(alen);
  vectorFromFile(vint2, "hist/INT-TO_DEVICE_ARRAY.intarray");
  vectorFromFile(vdouble2, "hist/DOUBLE-TO_DEVICE_ARRAY.doublearray");
  CHECK_WITH_TIMEOUT(testArrayContent1<int>(vint2, "//INT/MY_RENAMED_INTARRAY", 150, 1) == true);
  CHECK_WITH_TIMEOUT(testArrayContent1<double>(vdouble2, "//DOUBLE/FROM_DEVICE_ARRAY", 250.3, 1) == true);
}
