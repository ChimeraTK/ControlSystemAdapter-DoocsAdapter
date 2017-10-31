#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>
#include "serverBasedTestTools.h"

ReferenceTestApplication referenceTestApplication("serverTestGlobalTurnOnOffHistory");

// declare that we have some thing like a doocs server. is is linked from the doocs lib, but there is no header.
extern int eq_server(int, char **);

//#include <limits>
//#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

/// Check that all expected variables are there.
void testVariableExistence(){
  // run update once to make sure the server is up and running
  sleep(1);

  // the stuff with default. We are lazy and put the integer types as we have to list D_double
  // and D_float separately anyway if we don't want to do meta-programming
  for (auto & location : {"SHORT", "USHORT", "CHAR", "UCHAR", "INT", "UINT"}){
    std::cout << "testing " << location << std::endl;
    checkDoocsProperty<D_int>(std::string("//")+location+"/DATA_TYPE_CONSTANT", false, false);
    checkDoocsProperty<D_int>(std::string("//")+location+"/FROM_DEVICE_SCALAR", false, false);
    checkDoocsProperty<D_int>(std::string("//")+location+"/TO_DEVICE_SCALAR", false ,true);
  }
  for (auto & nameWriteable : { std::make_pair("CONSTANT_ARRAY", false), {"FROM_DEVICE_ARRAY", false}, {"TO_DEVICE_ARRAY", true}}){ 
    checkDoocsProperty<D_intarray>(std::string("//INT/")+nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_intarray>(std::string("//UINT/")+nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_bytearray>(std::string("//CHAR/")+nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_bytearray>(std::string("//UCHAR/")+nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_shortarray>(std::string("//SHORT/")+nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_shortarray>(std::string("//USHORT/")+nameWriteable.first, false, nameWriteable.second);
  }
  
  std::cout << "testing DOUBLE" << std::endl;
  checkDoocsProperty<D_doublearray>("//DOUBLE/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_doublearray>("//DOUBLE/TO_DEVICE_ARRAY", true );
  checkDoocsProperty<D_double>("//DOUBLE/DATA_TYPE_CONSTANT", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/TO_DEVICE_SCALAR", false, true);

  std::cout << "testing FLOAT" << std::endl;
  checkDoocsProperty<D_floatarray>("//FLOAT/CONSTANT_ARRAY" , false, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/FROM_DEVICE_ARRAY", false, false );
  checkDoocsProperty<D_floatarray>("//FLOAT/TO_DEVICE_ARRAY" );
  checkDoocsProperty<D_float>("//FLOAT/DATA_TYPE_CONSTANT", false, false );
  checkDoocsProperty<D_float>("//FLOAT/FROM_DEVICE_SCALAR", false, false);
  checkDoocsProperty<D_float>("//FLOAT/TO_DEVICE_SCALAR", true, true);

}

// due to the doocs server thread you can only have one test suite
class GlobalTurnOnOffHistoryServerTestSuite : public test_suite {
public:
  GlobalTurnOnOffHistoryServerTestSuite(int argc, char* argv[])
    : test_suite("GlobalTurnOnOffHistory server test suite") ,
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
  framework::master_test_suite().p_name.value = "GlobalTurnOnOffHistory server test suite";
  return new GlobalTurnOnOffHistoryServerTestSuite(argc, argv);
}
