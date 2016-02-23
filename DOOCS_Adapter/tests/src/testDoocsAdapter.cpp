// Define a name for the test module.
#define BOOST_TEST_MODULE DoocsAdapterTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "emptyServerFunctions.h"

#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/ProcessScalar.h>
#include <ControlSystemAdapter/SynchronizationDirection.h>

using namespace boost::unit_test_framework;
using namespace mtca4u;

// expose the properties vector for testing
class TestableDoocsAdapter: public DoocsAdapter{
public:
  TestableDoocsAdapter(EqFct *eqFct): DoocsAdapter(eqFct){
  }
  std::vector< boost::shared_ptr<D_fct> > & getDoocsProperties(){
    return _doocsProperties;
  }
};

struct BusinessLogic{
  ProcessScalar<int>::SharedPtr toDeviceInt;
  ProcessScalar<int>::SharedPtr fromDeviceInt;

  BusinessLogic(boost::shared_ptr<mtca4u::DevicePVManager> & pvManager)
    : toDeviceInt(pvManager->createProcessScalar<int>(
	controlSystemToDevice, "TO_DEVICE_INT") ),
      fromDeviceInt(pvManager->createProcessScalar<int>(
	deviceToControlSystem, "FROM_DEVICE_INT") ){
  }
};

BOOST_AUTO_TEST_SUITE( DoocsAdapterRestSuite )

BOOST_AUTO_TEST_CASE( testDoocsAdapter ) {
  TestableDoocsAdapter doocsAdapter(NULL);
  BusinessLogic businessLogic( doocsAdapter.getDevicePVManager() );

  // Test that the right number of properties is created.
  // Currently the vector is still empty.
  BOOST_CHECK( doocsAdapter.getDoocsProperties().empty() );
  doocsAdapter.registerProcessVariablesInDoocs();
  BOOST_REQUIRE( doocsAdapter.getDoocsProperties().size() == 2 );

  // extract the two properties and test the send/receive functionality

  // We do extraction by name and put them into a map. For convenience we use
  // raw pointers. This would not be possible in a real application because
  // the vector with the properties is not exposed.
  std::map< std::string, D_int *> doocsProperties;
  
  for (size_t i = 0; i < doocsAdapter.getDoocsProperties().size(); ++i){
    D_int * doocsInt = 
      dynamic_cast< D_int * >(doocsAdapter.getDoocsProperties()[i].get() );
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
  doocsAdapter.receiveAll();
  BOOST_CHECK( doocsProperties["FROM_DEVICE_INT "]->value() == 12);

  *(businessLogic.fromDeviceInt) = 15;
  businessLogic.fromDeviceInt->send();
  doocsAdapter.receiveAll();
  BOOST_CHECK( doocsProperties["FROM_DEVICE_INT "]->value() == 15);
}

BOOST_AUTO_TEST_SUITE_END()
