// Define a name for the test module.
#define BOOST_TEST_MODULE CSAdapterEqFct
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include "CSAdapterEqFct.h"
#include "emptyServerFunctions.h"
#include "set_doocs_value.h"
#include "DoocsAdapter.h"
#include "getAllVariableNames.h"
#include "VariableMapper.h"

#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <ChimeraTK/NDRegisterAccessor.h>
#include <ChimeraTK/ControlSystemAdapter/SynchronizationDirection.h>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

// expose the properties vector for testing
class TestableCSAdapterEqFct: public CSAdapterEqFct{
public:
  TestableCSAdapterEqFct(int fctCode,
                         boost::shared_ptr<ControlSystemPVManager> controlSystemPVManager,
                         boost::shared_ptr<DoocsUpdater> const & updater,
                         std::string fctName):
    CSAdapterEqFct(fctCode, controlSystemPVManager, updater, fctName){
  }
  std::vector< boost::shared_ptr<D_fct> > & getDoocsProperties(){
    return doocsProperties_;
  }
};

struct BusinessLogic{
  boost::shared_ptr< ChimeraTK::NDRegisterAccessor<int> > toDeviceInt;
  boost::shared_ptr< ChimeraTK::NDRegisterAccessor<int> > fromDeviceInt;

  BusinessLogic(boost::shared_ptr<ChimeraTK::DevicePVManager> const & pvManager)
  : toDeviceInt(pvManager->createProcessArray<int>(controlSystemToDevice, "test/TO_DEVICE/INT",1)),
    fromDeviceInt(pvManager->createProcessArray<int>(deviceToControlSystem, "test/FROM_DEVICE/INT",1))
  {}
};

BOOST_AUTO_TEST_SUITE( CSAdapterEqFctTestSuite )

BOOST_AUTO_TEST_CASE( testCSAdapterEqFct ) {
  DoocsAdapter doocsAdapter;
  // first create the business logic, which registers the PVs to the manager
  BusinessLogic businessLogic( doocsAdapter.getDevicePVManager() );
  // we also have to initialise the mapping (direct 1:1 from the input without xml)
  auto csManager = doocsAdapter.getControlSystemPVManager();
  VariableMapper::getInstance().directImport( getAllVariableNames(csManager ) );
  // after that create the EqFct
  auto updater = boost::make_shared<DoocsUpdater>();
  TestableCSAdapterEqFct eqFct(42, csManager, updater, "test");

  // Test that the right number of properties is created.
  // Currently the vector is still empty.
  BOOST_CHECK( eqFct.fct_code() == 42 );
  BOOST_REQUIRE( eqFct.getDoocsProperties().size() == 2 );

  // extract the two properties and test the send/receive functionality

  // We do extraction by name and put them into a map. For convenience we use
  // raw pointers. This would not be possible in a real application because
  // the vector with the properties is not exposed.
  std::map< std::string, D_int *> doocsProperties;

  for (size_t i = 0; i < eqFct.getDoocsProperties().size(); ++i){
    D_int *doocsInt = dynamic_cast<D_int*>(eqFct.getDoocsProperties()[i].get());
    BOOST_REQUIRE(doocsInt);
    doocsProperties[doocsInt->property_name()] = doocsInt;
  }
  // note: doocs property names always have a space (and a comment which can
  // be empty, but the space is always there)"
  BOOST_REQUIRE( doocsProperties.find("TO_DEVICE.INT ") != doocsProperties.end() );
  BOOST_REQUIRE( doocsProperties.find("FROM_DEVICE.INT ") != doocsProperties.end() );

  // write once and check
  set_doocs_value(*(doocsProperties["TO_DEVICE.INT "]),13);
  businessLogic.toDeviceInt->readNonBlocking();
  BOOST_CHECK_EQUAL( businessLogic.toDeviceInt->accessData(0), 13 );
  // change and observe the change in the device
  set_doocs_value(*(doocsProperties["TO_DEVICE.INT "]),14);
  businessLogic.toDeviceInt->readNonBlocking();
  BOOST_CHECK_EQUAL( businessLogic.toDeviceInt->accessData(0), 14 );

  // and the other direction
  businessLogic.fromDeviceInt->accessData(0) = 12;
  businessLogic.fromDeviceInt->write();
  updater->update();
  BOOST_CHECK_EQUAL( doocsProperties["FROM_DEVICE.INT "]->value(), 12);

  businessLogic.fromDeviceInt->accessData(0) = 15;
  businessLogic.fromDeviceInt->write();
  updater->update();
  BOOST_CHECK_EQUAL( doocsProperties["FROM_DEVICE.INT "]->value(), 15);
}

BOOST_AUTO_TEST_CASE( testWithMapping ) {
  // This test is a bit redundant because the mapping is tested separately. Just to see that it
  // works after integration.

  // This is basically a stripped down copy of the test before, so we leave out all comments
  // and just comment what is different.

  DoocsAdapter doocsAdapter;
  BusinessLogic businessLogic( doocsAdapter.getDevicePVManager() );

  // Initialse the mapping with an xml file.
  auto csManager = doocsAdapter.getControlSystemPVManager();

  VariableMapper::getInstance().prepareOutput( "EqFctTest.xml", getAllVariableNames(csManager ) );

  // in the mapping two locations are created
  auto updater = boost::make_shared<DoocsUpdater>();
  TestableCSAdapterEqFct toDeviceEqFct(42, csManager, updater, "test.TO_DEVICE");
  TestableCSAdapterEqFct fromDeviceEqFct(42, csManager, updater, "test.FROM_DEVICE");

  BOOST_REQUIRE( toDeviceEqFct.getDoocsProperties().size() == 1 );
  BOOST_REQUIRE( fromDeviceEqFct.getDoocsProperties().size() == 1 );

  // extract the two properties and check the name
  std::map< std::string, D_int *> doocsProperties;

  // both properties are called int, but are in different locations
  D_int *doocsInt = dynamic_cast<D_int*>(toDeviceEqFct.getDoocsProperties()[0].get());
  BOOST_REQUIRE( std::string(doocsInt->property_name()) == "INT ");
  doocsInt = dynamic_cast<D_int*>(fromDeviceEqFct.getDoocsProperties()[0].get());
  BOOST_REQUIRE( std::string(doocsInt->property_name()) == "INT ");
}

BOOST_AUTO_TEST_SUITE_END()
