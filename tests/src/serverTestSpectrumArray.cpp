#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>
#include "serverBasedTestTools.h"

ReferenceTestApplication referenceTestApplication("serverTestSpectrumArray");

// declare that we have some thing like a doocs server. is is linked from the doocs lib, but there is no header.
extern int eq_server(int, char **);

//#include <limits>
//#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

// use boost meta-programming to use test case templates
// The list of types is an mpl type
//typedef boost::mpl::list<int32_t, uint32_t,
//			 int16_t, uint16_t,
//			 int8_t, uint8_t,
//			 float, double> simple_test_types;

/// Check that all expected variables are there.
void testReadWrite(){
  // halt the test application tread 
  referenceTestApplication.initialiseManualLoopControl();
  // run update once to make sure the server is up and running, and empty the queues from the initial
  // values.
  sleep(1);

  // prepare the x-axis for the float array (we are using the float and double scalar)
  DoocsServerTestHelper::doocsSet("//FLOAT/START",12.3);
  DoocsServerTestHelper::doocsSet("//FLOAT/INCREMENT", 1.6);
  referenceTestApplication.runMainLoopOnce();
  sleep(1);
  
  checkSpectrum("//INT/TO_DEVICE_ARRAY");
  checkSpectrum("//DOUBLE/TO_DEVICE_ARRAY");
  checkSpectrum("//UINT/TO_DEVICE_ARRAY");
  checkSpectrum("//INT/MY_RENAMED_INTARRAY",true, false);
  checkSpectrum("//DOUBLE/FROM_DEVICE_ARRAY",true, false, 123., 0.56);
  checkSpectrum("//FLOAT/TO_DEVICE_ARRAY",true, true, 12.3, 1.6);
  checkSpectrum("//FLOAT/FROM_DEVICE_ARRAY",true, false, 12.3, 1.6);
  checkSpectrum("//UINT/FROM_DEVICE_ARRAY",true, false, 12.3, 1.6);

  // check that the "short array" is of type long
  checkDataType("//SHORT/MY_RETYPED_SHORT_ARRAY", DATA_A_LONG);
  
  DoocsServerTestHelper::doocsSetSpectrum("//INT/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149} ); 
  DoocsServerTestHelper::doocsSetSpectrum("//DOUBLE/TO_DEVICE_ARRAY", {240.3, 241.3, 242.3, 243.3, 244.3, 245.3, 246.3, 247.3, 248.3, 249.3} ); 
  
  // running update now does not change anything, the application has not acted yet
  sleep(1);

  auto notIntArray = DoocsServerTestHelper::doocsGetArray<float>("//INT/MY_RENAMED_INTARRAY");
  for (auto val : notIntArray){
    BOOST_CHECK( std::fabs(val) < 0.001 );
  }
  auto notFloatArray = DoocsServerTestHelper::doocsGetArray<float>("//DOUBLE/FROM_DEVICE_ARRAY");
  for (auto val : notFloatArray){
    BOOST_CHECK( std::fabs(val) < 0.001 );
  }
  
  // run the application loop. Still no changes until we run the doocs server update
  referenceTestApplication.runMainLoopOnce();

  notIntArray = DoocsServerTestHelper::doocsGetArray<float>("//INT/MY_RENAMED_INTARRAY");
  for (auto val : notIntArray){
    BOOST_CHECK( std::fabs(val) < 0.001 );
  }
  notFloatArray = DoocsServerTestHelper::doocsGetArray<float>("//DOUBLE/FROM_DEVICE_ARRAY");
  for (auto val : notFloatArray){
    BOOST_CHECK( std::fabs(val) < 0.001 );
  }
  
  // now finally after the next update we should see the new data in doocs
  sleep(1);

  notIntArray = DoocsServerTestHelper::doocsGetArray<float>("//INT/MY_RENAMED_INTARRAY");
  int testVal = 140;
  for (auto val : notIntArray){
    BOOST_CHECK( std::fabs(val - testVal++) < 0.001 );
  }
  notFloatArray = DoocsServerTestHelper::doocsGetArray<float>("//DOUBLE/FROM_DEVICE_ARRAY");
  float floatTestVal = 240.3;
  for (auto val : notFloatArray){
    BOOST_CHECK( std::fabs(val - floatTestVal++) < 0.001 );
  }

}

void testPropertyDoesNotExist(std::string addressString){
  EqAdr ad;
  EqData ed, res;
  // obtain location pointer
  ad.adr(addressString.c_str());
  EqFct *eqFct = eq_get(&ad);
  BOOST_REQUIRE_MESSAGE(eqFct != NULL, "Could not get location for property.");
  // obtain value
  eqFct->lock();
  eqFct->get(&ad,&ed,&res);
  eqFct->unlock();
  // check for errors
  ///todo FIXME: check for the correct error code
  BOOST_CHECK_MESSAGE(res.error() != 0, std::string("Could read from ")+ addressString +", but it should not be there");
}

void testPropertiesDontExist(){
  testPropertyDoesNotExist("//FLOAT/FROM_DEVICE_SCALAR");
  testPropertyDoesNotExist("//DOUBLE/FROM_DEVICE_SCALAR");
}


// due to the doocs server thread you can only have one test suite
class DoocsServerTestSuite : public test_suite {
public:
  DoocsServerTestSuite(int argc, char* argv[])
    : test_suite("Spectrum and array server test suite") ,
      doocsServerThread(eq_server, argc, argv)
  {
    doocsServerThread.detach();
    add( BOOST_TEST_CASE( &testReadWrite ) );
    add( BOOST_TEST_CASE( &testPropertiesDontExist ) );
  }
  virtual ~DoocsServerTestSuite(){
    referenceTestApplication.releaseManualLoopControl();
  }
protected:
  std::thread doocsServerThread;
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  framework::master_test_suite().p_name.value = "Spectrum and array server test suite";
  return new DoocsServerTestSuite(argc, argv);
}
