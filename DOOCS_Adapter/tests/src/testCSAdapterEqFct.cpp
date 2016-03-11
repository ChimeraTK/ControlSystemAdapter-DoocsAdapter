// Define a name for the test module.
#define BOOST_TEST_MODULE CSAdapterEqFct
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include "CSAdapterEqFct.h"
#include "emptyServerFunctions.h"
#include "DoocsAdapter.h"

#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/ProcessScalar.h>
#include <ControlSystemAdapter/SynchronizationDirection.h>

using namespace boost::unit_test_framework;
using namespace mtca4u;

// expose the properties vector for testing
class TestableCSAdapterEqFct: public CSAdapterEqFct{
public:
  TestableCSAdapterEqFct(const char * fctName, int fctCode,
      boost::shared_ptr<ControlSystemPVManager> controlSystemPVManager):
    CSAdapterEqFct(fctName, fctCode, controlSystemPVManager){
  }
  std::vector< boost::shared_ptr<D_fct> > & getDoocsProperties(){
    return doocsProperties_;
  }
};

struct BusinessLogic{
  ProcessScalar<int>::SharedPtr toDeviceInt;
  ProcessScalar<int>::SharedPtr fromDeviceInt;

  BusinessLogic(boost::shared_ptr<mtca4u::DevicePVManager> const & pvManager)
    : toDeviceInt(pvManager->createProcessScalar<int>(
	controlSystemToDevice, "TO_DEVICE_INT") ),
      fromDeviceInt(pvManager->createProcessScalar<int>(
	deviceToControlSystem, "FROM_DEVICE_INT") ){
  }
};

BOOST_AUTO_TEST_SUITE( CSAdapterEqFctTestSuite )

BOOST_AUTO_TEST_CASE( testCSAdapterEqFct ) {
  DoocsAdapter doocsAdapter;
  // first create the business logic, which registers the PVs to the manager
  BusinessLogic businessLogic( doocsAdapter.getDevicePVManager() );
  // after that create the EqFct
  TestableCSAdapterEqFct eqFct("NAME = test", 42, doocsAdapter.getControlSystemPVManager());

  // Test that the right number of properties is created.
  // Currently the vector is still empty.
  BOOST_CHECK( eqFct.fct_code() == 42);
  BOOST_REQUIRE( eqFct.getDoocsProperties().size() == 2 );

  // extract the two properties and test the send/receive functionality

  // We do extraction by name and put them into a map. For convenience we use
  // raw pointers. This would not be possible in a real application because
  // the vector with the properties is not exposed.
  std::map< std::string, D_int *> doocsProperties;
  
  for (size_t i = 0; i < eqFct.getDoocsProperties().size(); ++i){
    D_int * doocsInt = 
      dynamic_cast< D_int * >(eqFct.getDoocsProperties()[i].get() );
    BOOST_REQUIRE(doocsInt);
    doocsProperties[doocsInt->property_name()] = doocsInt;
  }
  // note: doocs property names always have a space (and a comment which can
  // be empty, but the space is always there)"
  BOOST_REQUIRE( doocsProperties.find("TO_DEVICE_INT ") != doocsProperties.end() );
  BOOST_REQUIRE( doocsProperties.find("FROM_DEVICE_INT ") != doocsProperties.end() );

  // write once and check
  doocsProperties["TO_DEVICE_INT "]->set_value(13);
  businessLogic.toDeviceInt->receive();
  BOOST_CHECK( *(businessLogic.toDeviceInt) == 13 );
  // change and observe the change in the device
  doocsProperties["TO_DEVICE_INT "]->set_value(14);
  businessLogic.toDeviceInt->receive();
  BOOST_CHECK( *(businessLogic.toDeviceInt) == 14 );
  
  // and the other direction
  *(businessLogic.fromDeviceInt) = 12;
  businessLogic.fromDeviceInt->send();
  eqFct.update();
  BOOST_CHECK( doocsProperties["FROM_DEVICE_INT "]->value() == 12);

  *(businessLogic.fromDeviceInt) = 15;
  businessLogic.fromDeviceInt->send();
  eqFct.update();
  BOOST_CHECK( doocsProperties["FROM_DEVICE_INT "]->value() == 15);
}

BOOST_AUTO_TEST_SUITE_END()
