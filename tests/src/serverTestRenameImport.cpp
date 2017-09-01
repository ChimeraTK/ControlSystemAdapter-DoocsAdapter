#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>

ReferenceTestApplication referenceTestApplication("serverTestRenameImport");

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
  // run update once to make sure the server is up and running
  std::cout << "running update once " << std::endl;
  DoocsServerTestHelper::runUpdate();
  std::cout << "ran update once, let's test " << std::endl;

  for (auto const location : { "CHAR", "UCHAR", "UINT", "USHORT"} ){
    for (auto const property : { "CONSTANT_ARRAY", "FROM_DEVICE_ARRAY", "TO_DEVICE_ARRAY "} ){
      // if this throws the property does not exist. we should always be able to read"
      BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>( (std::string("//")+location+"/"+ property).c_str() ));
    }
    for (auto const property : { "DATA_TYPE_CONSTANT", "FROM_DEVICE_SCALAR", "TO_DEVICE_SCALAR"} ){
      // if this throws the property does not exist. we should always be able to read"
      BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>( (std::string("//")+location+"/"+ property).c_str() ));
    }
  }
  
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//MY_RENAMED_INTEGER_LOCATION/RENAMED.CONST_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//MY_RENAMED_INTEGER_LOCATION/FROM_DEVICE_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//MY_RENAMED_INTEGER_LOCATION/TO_DEVICE_ARRAY") );
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//MY_RENAMED_INTEGER_LOCATION/DATA_TYPE_CONSTANT") == -4);
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//MY_RENAMED_INTEGER_LOCATION/FROM_DEVICE_SCALAR") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//MY_RENAMED_INTEGER_LOCATION/TO_DEVICE_SCALAR") );

  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//SHORT/myStuff.CONSTANT_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//SHORT/myStuff.FROM_DEVICE_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//SHORT/myStuff.TO_DEVICE_ARRAY") );
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//SHORT/myStuff.DATA_TYPE_CONSTANT") == -2);
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//SHORT/myStuff.FROM_DEVICE_SCALAR") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//SHORT/myStuff.TO_DEVICE_SCALAR") );

  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//DOUBLE/CONSTANT_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//DOUBLE/FROM_DEVICE_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//DOUBLE/TO_DEVICE_ARRAY") );
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<double>("//DOUBLE/RENAMED_CONSTANT") == 1./8.);
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//DOUBLE/FROM_DEVICE_SCALAR") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//DOUBLE/DOUBLE.TO_DEVICE_SCALAR") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//DOUBLE/I_AM_A_FLOAT_SCALAR") );
  // we moved one float scalar to the double location, so we cannot check for it in the loop above
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//FLOAT/CONSTANT_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//FLOAT/FROM_DEVICE_ARRAY") );
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGetArray<int>("//FLOAT/TO_DEVICE_ARRAY") );
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<float>("//FLOAT/DATA_TYPE_CONSTANT") == 1./4.);
  BOOST_CHECK_NO_THROW( DoocsServerTestHelper::doocsGet<int>("//FLOAT/FROM_DEVICE_SCALAR") );
}

// due to the doocs server thread you can only have one test suite
class RenameImportServerTestSuite : public test_suite {
public:
  RenameImportServerTestSuite(int argc, char* argv[])
    : test_suite("RenameImport server test suite") ,
      doocsServerThread(eq_server, argc, argv)
  {
    doocsServerThread.detach();
    add( BOOST_TEST_CASE( &testVariableExistence ) );
  }

protected:
  std::thread doocsServerThread;
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  framework::master_test_suite().p_name.value = "RenameImport server test suite";
  return new RenameImportServerTestSuite(argc, argv);
}
