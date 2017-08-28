#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>

ReferenceTestApplication referenceTestApplication("testPlainVariableCreation");

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
  BOOST_ERROR("Not implemented yet");
}

// due to the doocs server thread you can only have one test suite
class PlainVariableCreationTestSuite : public test_suite {
public:
  PlainVariableCreationTestSuite(int argc, char* argv[])
    : test_suite("PlainVariableCreation test suite") ,
      doocsServerThread(eq_server, argc, argv)
  {
    add( BOOST_TEST_CASE( &testVariableExistence ) );
  }
  ~PlainVariableCreationTestSuite(){
    // shut down the doocs server correctly 
    DoocsServerTestHelper::shutdown();
    doocsServerThread.join();
  }

protected:
  std::thread doocsServerThread;
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  framework::master_test_suite().p_name.value = "PlainVariableCreation test suite";
  return new PlainVariableCreationTestSuite(argc, argv);
}
