#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>
#include "basenameFromAddress.h"

// these constants don't exist in DOOCS, although they should. We define them here for better code readability
static const int ACCESS_RO = 0; // read only
static const int ACCESS_RW = 1; // read/write

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


template<class DOOCS_T>
void checkHistory(DOOCS_T * property, bool expected_has_history){
  bool has_history = property->get_histPointer();
  BOOST_CHECK_MESSAGE( has_history == expected_has_history, "History on/off wrong for "+ property->basename() +". Should be " + (expected_has_history?"true":"false"));
}

template<>
void checkHistory(D_spectrum * /*property*/, bool){
  // nothing to do, spectra don't have history
}

template<class DOOCS_T>
void checkDoocsProperty(std::string const & propertyAddress, bool expected_has_history =  true, bool expected_is_writeable =true){
  // copied from DoocsServerTestHelper::doocsGet
  EqAdr ad;
  EqData ed, res;
  // obtain location pointer
  ad.adr(propertyAddress.c_str());
  EqFct *eqFct = eq_get(&ad);
  BOOST_REQUIRE_MESSAGE( eqFct, "Could not get location for property "+propertyAddress);

  auto propertyName = basenameFromAddress(propertyAddress);
  DOOCS_T * property = dynamic_cast<DOOCS_T *>(eqFct->find_property(propertyName));
  BOOST_REQUIRE_MESSAGE(property, "Could not find property " + propertyName + " (address "<< propertyAddress <<"), or property has unexpected type.");

  checkHistory(property, expected_has_history);

  std::stringstream errorMessage;
  errorMessage << "Access rights not correct for '" << propertyAddress << "': access word is "
               << property->get_access() << ", expected " << (expected_is_writeable?ACCESS_RW:ACCESS_RO);
  if (expected_is_writeable){
    BOOST_CHECK_MESSAGE( property->get_access() == ACCESS_RW , errorMessage.str());
  }else{
    BOOST_CHECK( property->get_access() == ACCESS_RO );    
  }
}

/// Check that all expected variables are there.
void testVariableExistence(){
  // run update once to make sure the server is up and running
  DoocsServerTestHelper::runUpdate();

  checkDoocsProperty<D_spectrum>("//MY_RENAMED_INTEGER_LOCATION/RENAMED.CONST_ARRAY", true, false);
  checkDoocsProperty<D_spectrum>("//MY_RENAMED_INTEGER_LOCATION/FROM_DEVICE_ARRAY" , true, false);
  checkDoocsProperty<D_spectrum>("//MY_RENAMED_INTEGER_LOCATION/TO_DEVICE_ARRAY", true, true );
  checkDoocsProperty<D_int>("//MY_RENAMED_INTEGER_LOCATION/DATA_TYPE_CONSTANT", true, false);
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//MY_RENAMED_INTEGER_LOCATION/DATA_TYPE_CONSTANT") == -4);
  checkDoocsProperty<D_int>("//MY_RENAMED_INTEGER_LOCATION/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//MY_RENAMED_INTEGER_LOCATION/TO_DEVICE_SCALAR", true ,true);

  checkDoocsProperty<D_spectrum>("//SHORT/myStuff.CONSTANT_ARRAY" , true, false);
  checkDoocsProperty<D_spectrum>("//SHORT/myStuff.FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_spectrum>("//SHORT/myStuff.TO_DEVICE_ARRAY" );
  checkDoocsProperty<D_int>("//SHORT/myStuff.DATA_TYPE_CONSTANT", true, false);
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//SHORT/myStuff.DATA_TYPE_CONSTANT") == -2);
  checkDoocsProperty<D_int>("//SHORT/myStuff.FROM_DEVICE_SCALAR", true, false);

  checkDoocsProperty<D_int>("//CHERRY_PICKED/TO_DEVICE_SHORT");

  checkDoocsProperty<D_spectrum>("//DOUBLE/CONSTANT_ARRAY" , true, false);
  checkDoocsProperty<D_spectrum>("//DOUBLE/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_spectrum>("//DOUBLE/TO_DEVICE_ARRAY" );
  checkDoocsProperty<D_double>("//DOUBLE/RENAMED_CONSTANT", false, false);
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<double>("//DOUBLE/RENAMED_CONSTANT") == 1./8.);
  checkDoocsProperty<D_double>("//DOUBLE/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/DOUBLE.TO_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_float>("//DOUBLE/I_AM_A_FLOAT_SCALAR");

  // we moved one float scalar to the double location, so we cannot check for it in the loop above
  checkDoocsProperty<D_spectrum>("//FLOAT/CONSTANT_ARRAY" , true, false);
  checkDoocsProperty<D_spectrum>("//FLOAT/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_spectrum>("//FLOAT/TO_DEVICE_ARRAY" );
  checkDoocsProperty<D_float>("//FLOAT/DATA_TYPE_CONSTANT", true, false );
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<float>("//FLOAT/DATA_TYPE_CONSTANT") == 1./4.);
  checkDoocsProperty<D_float>("//FLOAT/FROM_DEVICE_SCALAR", true, false);

  checkDoocsProperty<D_spectrum>("//UINT/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_spectrum>("//UINT/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_spectrum>("//UINT/TO_DEVICE_ARRAY" );
  checkDoocsProperty<D_int>("//UINT/DATA_TYPE_CONSTANT", true, false);
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//UINT/DATA_TYPE_CONSTANT") == 4);
  checkDoocsProperty<D_int>("//UINT/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//UINT/TO_DEVICE_SCALAR");

  checkDoocsProperty<D_spectrum>("//USHORT/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_spectrum>("//USHORT/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_spectrum>("//USHORT/TO_DEVICE_ARRAY");
  checkDoocsProperty<D_int>("//USHORT/DATA_TYPE_CONSTANT", false, false );
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//USHORT/DATA_TYPE_CONSTANT") == 2);
  checkDoocsProperty<D_int>("//USHORT/FROM_DEVICE_SCALAR", false, false);
  checkDoocsProperty<D_int>("//USHORT/TO_DEVICE_SCALAR", true, true);

  checkDoocsProperty<D_spectrum>("//UCHAR/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_spectrum>("//UCHAR/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_spectrum>("//UCHAR/TO_DEVICE_ARRAY" , true, false);
  checkDoocsProperty<D_int>("//UCHAR/DATA_TYPE_CONSTANT", true, false);
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//UCHAR/DATA_TYPE_CONSTANT") == 1);
  checkDoocsProperty<D_int>("//UCHAR/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//UCHAR/TO_DEVICE_SCALAR");

  checkDoocsProperty<D_spectrum>("//CHAR/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_spectrum>("//CHAR/FROM_DEVICE_ARRAY", true, false );
  checkDoocsProperty<D_spectrum>("//CHAR/TO_DEVICE_ARRAY" );
  checkDoocsProperty<D_int>("//CHAR/DATA_TYPE_CONSTANT", true, false);
  BOOST_CHECK( DoocsServerTestHelper::doocsGet<int>("//CHAR/DATA_TYPE_CONSTANT") == -1);
  checkDoocsProperty<D_int>("//CHAR/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//CHAR/TO_DEVICE_SCALAR");
}

// due to the doocs server thread you can only have one test suite
class RenameImportServerTestSuite : public test_suite {
public:
  RenameImportServerTestSuite(int argc, char* argv[])
    : test_suite("RenameImport server test suite") ,
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
  framework::master_test_suite().p_name.value = "RenameImport server test suite";
  return new RenameImportServerTestSuite(argc, argv);
}