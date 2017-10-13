#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>
#include "serverBasedTestTools.h"

ReferenceTestApplication referenceTestApplication("serverTestGlobalTurnOnOffWriteable");

// declare that we have some thing like a doocs server. is is linked from the doocs lib, but there is no header.
extern int eq_server(int, char **);

//#include <limits>
//#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

/// Check that all expected variables are there.
void testVariableExistence(){
  // run update once to make sure the server is up and running
  DoocsServerTestHelper::runUpdate();

  // the stuff with default. We are lazy and put the integer types as we have to list D_double
  // and D_float separately anyway if we don't want to do meta-programming
  for (auto & location : {"SHORT", "USHORT", "CHAR", "UCHAR", "INT", "UINT"}){
    std::cout << "testing " << location << std::endl;
    checkDoocsProperty<D_int>(std::string("//")+location+"/DATA_TYPE_CONSTANT", true, false);
    checkDoocsProperty<D_int>(std::string("//")+location+"/FROM_DEVICE_SCALAR", true, false);
    checkDoocsProperty<D_int>(std::string("//")+location+"/TO_DEVICE_SCALAR", true, false);
  }
  for (auto & name : { "CONSTANT_ARRAY", "FROM_DEVICE_ARRAY", "TO_DEVICE_ARRAY"}){ 
    checkDoocsProperty<D_intarray>(std::string("//INT/")+name, true, false);
    checkDoocsProperty<D_intarray>(std::string("//UINT/")+name, true, false);
    checkDoocsProperty<D_bytearray>(std::string("//CHAR/")+name, true, false);
    checkDoocsProperty<D_bytearray>(std::string("//UCHAR/")+name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//SHORT/")+name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//USHORT/")+name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//SHORT/")+name, true, false);
    checkDoocsProperty<D_shortarray>(std::string("//USHORT/")+name, true, false);
  }
  
  std::cout << "testing DOUBLE" << std::endl;
  checkDoocsProperty<D_doublearray>("//DOUBLE/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_doublearray>("//DOUBLE/TO_DEVICE_ARRAY", true, true );
  checkDoocsProperty<D_double>("//DOUBLE/DATA_TYPE_CONSTANT", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/TO_DEVICE_SCALAR", true, false);

  std::cout << "testing FLOAT" << std::endl;
  checkDoocsProperty<D_floatarray>("//FLOAT/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_floatarray>("//FLOAT/TO_DEVICE_ARRAY", true, true);
  checkDoocsProperty<D_float>("//FLOAT/DATA_TYPE_CONSTANT", true, false );
  checkDoocsProperty<D_float>("//FLOAT/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_float>("//FLOAT/TO_DEVICE_SCALAR", true, false);

}

// due to the doocs server thread you can only have one test suite
class GlobalTurnOnOffWriteableServerTestSuite : public test_suite {
public:
  GlobalTurnOnOffWriteableServerTestSuite(int argc, char* argv[])
    : test_suite("GlobalTurnOnOffWriteable server test suite") ,
      doocsServerThread(eq_server, argc, argv)
  {
    doocsServerThread.detach();
    add( BOOST_TEST_CASE(&testVariableExistence) );
  }

protected:
  std::thread doocsServerThread;
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  framework::master_test_suite().p_name.value = "GlobalTurnOnOffWriteable server test suite";
  return new GlobalTurnOnOffWriteableServerTestSuite(argc, argv);
}
