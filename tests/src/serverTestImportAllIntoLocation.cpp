#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>
#include "DoocsAdapter.h"

ReferenceTestApplication referenceTestApplication("serverTestImportAllIntoLocation");

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
void testVariableExistence(){
  for (auto const directory : { "CHAR", "DOUBLE", "FLOAT", "INT", "SHORT", "UCHAR", "UINT", "USHORT"} ){
    for (auto const variable : { "CONSTANT_ARRAY", "FROM_DEVICE_ARRAY", "TO_DEVICE_ARRAY "} ){
      // if this throws the property does not exist. we should always be able to read"
      BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>( (std::string("//MASTER/")+directory+"."+ variable).c_str() ));
    }
    for (auto const variable : { "DATA_TYPE_CONSTANT", "FROM_DEVICE_SCALAR", "TO_DEVICE_SCALAR"} ){
      // if this throws the property does not exist. we should always be able to read"
      BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>( (std::string("//MASTER/")+directory+"."+ variable).c_str() ));
    }
  }
}

// due to the doocs server thread you can only have one test suite
class MyTestSuite : public test_suite {
public:
  MyTestSuite(int argc, char* argv[])
    : test_suite("ImportAllIntoLocation server test suite") ,
      doocsServerThread(eq_server, argc, argv)
  {
    // wait for doocs to start up before detaching the thread and continuing
    ChimeraTK::DoocsAdapter::waitUntilInitialised();
    doocsServerThread.detach();
    add( BOOST_TEST_CASE( &testVariableExistence ) );
  }

protected:
  std::thread doocsServerThread;
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  framework::master_test_suite().p_name.value = "ImportAllIntoLocation server test suite";
  return new MyTestSuite(argc, argv);
}
