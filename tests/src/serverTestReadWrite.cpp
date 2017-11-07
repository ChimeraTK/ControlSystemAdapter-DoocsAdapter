#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>
#include "serverBasedTestTools.h"
#include "DoocsAdapter.h"

ReferenceTestApplication referenceTestApplication("serverTestReadWrite");

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
  std::cout << "got the application main lock" << std::endl;
  
  // just a few tests before we start
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//INT/DATA_TYPE_CONSTANT") == -4 );
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//CHAR/DATA_TYPE_CONSTANT") == -1 );
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == 0 );
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//CHAR/FROM_DEVICE_SCALAR") == 0 );

  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_SCALAR", 42 );
  DoocsServerTestHelper::doocsSet<int>("//CHAR/TO_DEVICE_SCALAR", 44 );
  DoocsServerTestHelper::doocsSet<int>("//INT/TO_DEVICE_ARRAY", {140, 141, 142, 143, 144, 145, 146, 147, 148, 149} ); 
  
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == 0 );
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//CHAR/FROM_DEVICE_SCALAR") == 0 );

  // run the application loop. Still no changes until we run the doocs server update
  referenceTestApplication.runMainLoopOnce();
  
  // now finally after the next update we should see the new data in doocs
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//INT/FROM_DEVICE_SCALAR") == 42 );
  CHECK_WITH_TIMEOUT( DoocsServerTestHelper::doocsGet<int>("//CHAR/FROM_DEVICE_SCALAR") == 44 );

  // we have to get stuff as float in order to work with spectra
  auto intArray = DoocsServerTestHelper::doocsGetArray<float>("//INT/FROM_DEVICE_ARRAY");
  int testVal = 140;
  for (auto val : intArray){
    CHECK_WITH_TIMEOUT( std::fabs(val - testVal++) < 0.001 );
  }

  auto constArray = DoocsServerTestHelper::doocsGetArray<float>("//INT/CONSTANT_ARRAY");
  for (int i =0; i < int(constArray.size()); ++i){
    CHECK_WITH_TIMEOUT( std::fabs(constArray[i] - (-4*i*i)) < 0.001 ); // float check to compensate binary roundings errors
  }
  
  for (auto const location : { "CHAR", "DOUBLE", "FLOAT", "INT", "SHORT", "UCHAR", "UINT", "USHORT"} ){
    for (auto const property : { "CONSTANT_ARRAY", "FROM_DEVICE_ARRAY", "TO_DEVICE_ARRAY "} ){
      // if this throws the property does not exist. we should always be able to read"
      BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>( (std::string("//")+location+"/"+ property).c_str() ));
    }
    for (auto const property : { "DATA_TYPE_CONSTANT", "FROM_DEVICE_SCALAR", "TO_DEVICE_SCALAR"} ){
      // if this throws the property does not exist. we should always be able to read"
      BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>( (std::string("//")+location+"/"+ property).c_str() ));
    }
  }
}

// due to the doocs server thread you can only have one test suite
class DoocsServerTestSuite : public test_suite {
public:
  DoocsServerTestSuite(int argc, char* argv[])
    : test_suite("Read write test suite") ,
      doocsServerThread(eq_server, argc, argv)
  {
    // wait for doocs to start up before detaching the thread and continuing
    ChimeraTK::DoocsAdapter::waitUntilInitialised();
    doocsServerThread.detach();
    add( BOOST_TEST_CASE( &testReadWrite ) );
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
  framework::master_test_suite().p_name.value = "Read write server test suite";
  return new DoocsServerTestSuite(argc, argv);
}
