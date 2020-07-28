#define BOOST_TEST_MODULE serverTestSpectrumArray
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

// the array must have testStartValue+i at index i.
template<class T>
static bool testArrayContent(std::string const& propertyName, T testStartValue, T delta) {
  auto array = DoocsServerTestHelper::doocsGetArray<T>(propertyName);
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

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(testReadWrite) {
  // prepare the x-axis for the float array (we are using the float and double
  // scalar)
  DoocsServerTestHelper::doocsSet("//FLOAT/START", 12.3);
  DoocsServerTestHelper::doocsSet("//FLOAT/INCREMENT", 1.6);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  checkSpectrum("//INT/TO_DEVICE_ARRAY");
  checkSpectrum("//DOUBLE/TO_DEVICE_ARRAY");
  checkSpectrum("//UINT/TO_DEVICE_ARRAY");
  checkSpectrum("//INT/MY_RENAMED_INTARRAY", true, false);
  checkSpectrum("//DOUBLE/FROM_DEVICE_ARRAY", true, false, 123., 0.56);
  checkSpectrum("//FLOAT/TO_DEVICE_ARRAY", true, true, 12.3, 1.6);
  checkSpectrum("//FLOAT/FROM_DEVICE_ARRAY", true, false, 12.3, 1.6);
  checkSpectrum("//UINT/FROM_DEVICE_ARRAY", true, false, 12.3, 1.6);

  // check that the "short array" is of type long
  checkDataType("//SHORT/MY_RETYPED_SHORT_ARRAY", DATA_A_LONG);

  DoocsServerTestHelper::doocsSetSpectrum("//INT/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149});
  DoocsServerTestHelper::doocsSetSpectrum(
      "//DOUBLE/TO_DEVICE_ARRAY", {240.3, 241.3, 242.3, 243.3, 244.3, 245.3, 246.3, 247.3, 248.3, 249.3});

  // check the the control system side still sees 0 in all arrays. The
  // application has not reacted yet

  CHECK_WITH_TIMEOUT(testArrayContent<float>("//INT/MY_RENAMED_INTARRAY", 0, 0) == true);
  CHECK_WITH_TIMEOUT(testArrayContent<float>("//DOUBLE/FROM_DEVICE_ARRAY", 0, 0) == true);

  // run the application loop.
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  CHECK_WITH_TIMEOUT(testArrayContent<float>("//INT/MY_RENAMED_INTARRAY", 140, 1) == true);
  CHECK_WITH_TIMEOUT(testArrayContent<float>("//DOUBLE/FROM_DEVICE_ARRAY", 240.3, 1) == true);

  //  notIntArray =
  //  DoocsServerTestHelper::doocsGetArray<double>("//INT/MY_RENAMED_INTARRAY")
  //    ;
  //
  //  int testVal = 140;
  //  for (auto val : notIntArray){
  //    BOOST_CHECK( std::fabs(val - testVal++) < 0.001 );
  //  }
  //  notFloatArray =
  //  DoocsServerTestHelper::doocsGetArray<float>("//DOUBLE/FROM_DEVICE_ARRAY");
  //  float floatTestVal = 240.3;
  //  for (auto val : notFloatArray){
  //    BOOST_CHECK( std::fabs(val - floatTestVal++) < 0.001 );
  //  }
}

static void testPropertyDoesNotExist(std::string addressString) {
  EqAdr ad;
  EqData ed, res;
  // obtain location pointer
  ad.adr(addressString.c_str());
  EqFct* eqFct = eq_get(&ad);
  BOOST_REQUIRE_MESSAGE(eqFct != NULL, "Could not get location for property.");
  // obtain value
  eqFct->lock();
  eqFct->get(&ad, &ed, &res);
  eqFct->unlock();
  // check for errors
  /// todo FIXME: check for the correct error code
  BOOST_CHECK_MESSAGE(
      res.error() != 0, std::string("Could read from ") + addressString + ", but it should not be there");
}

BOOST_AUTO_TEST_CASE(testPropertiesDontExist) {
  testPropertyDoesNotExist("//FLOAT/FROM_DEVICE_SCALAR");
  testPropertyDoesNotExist("//DOUBLE/FROM_DEVICE_SCALAR");
}
