#define BOOST_TEST_MODULE IndependentControlCoreTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "IndependentControlCore.h"
#include <ControlSystemAdapter/PVManager.h>
#include <ControlSystemAdapter/StubControlSystemPVFactory.h>
#include <ControlSystemAdapter/ControlSystemPVManager.h>
using namespace mtca4u;

#include <climits>

// A struct which sets up the IndependentControlCore with the StubProcessVariableAdapter
struct IndependentControlCoreStubFixture{
  boost::shared_ptr<ControlSystemPVManager> controlSystemPVManager;
  boost::shared_ptr<DevicePVManager> devicePVManager;

  boost::shared_ptr<IndependentControlCore> controlCore;

  boost::shared_ptr< ControlSystemProcessScalar<int> > targetVoltage;
  boost::shared_ptr< ControlSystemProcessScalar<int> > monitorVoltage;

  std::list<ControlSystemProcessVariable::SharedPtr> toDeviceProcessVariables;
  std::list<ControlSystemProcessVariable::SharedPtr> fromDeviceProcessVariables;
  std::list<ControlSystemProcessVariable::SharedPtr> emptyPVList;

  IndependentControlCoreStubFixture(){
    std::pair< boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers =
      createPVManager( ControlSystemPVFactory::SharedPtr(new StubControlSystemPVFactory) );
    
    controlSystemPVManager = pvManagers.first;
    devicePVManager = pvManagers.second;

    IndependentControlCore::registerProcessVariables( devicePVManager );

    targetVoltage = controlSystemPVManager->getProcessScalar<int>("TARGET_VOLTAGE");
    toDeviceProcessVariables.push_back( targetVoltage );

    monitorVoltage = controlSystemPVManager->getProcessScalar<int>("MONITOR_VOLTAGE");
    fromDeviceProcessVariables.push_back( monitorVoltage );

    std::list<ControlSystemProcessVariable::SharedPtr> initialisationListToDevice;
    initialisationListToDevice.push_back( targetVoltage );
    initialisationListToDevice.push_back( monitorVoltage );

    targetVoltage->set(35);
    monitorVoltage->set(35);

    std::cout << "TEST: initial sync" << std::endl;
    boost::thread initialisationThread( boost::bind( &ControlSystemPVManager::synchronize,
 						     &(*controlSystemPVManager),
						     initialisationListToDevice, emptyPVList, 200 ) );
    // do not time out until the intialisation thread has set the variables (max. approx 33 minutes)
    while (true){
      devicePVManager->processSynchronization(LONG_MAX);//LONG_MAX,200);
      if( initialisationThread.try_join_for(boost::chrono::microseconds(20)) ){
	break;
      }
    }
    
    controlCore.reset( new IndependentControlCore( devicePVManager ) );
    
  }
  ~IndependentControlCoreStubFixture(){
    std::cout << "fixture destructor" << std::endl;    
  }
};

BOOST_FIXTURE_TEST_SUITE( IndependentControlCoreTestSuite, IndependentControlCoreStubFixture )

BOOST_AUTO_TEST_CASE( testInitialisation ){
  BOOST_CHECK( *targetVoltage == 35 );
  BOOST_CHECK( *monitorVoltage == 35 );
  
  //  boost::this_thread::sleep(boost::posix_time::seconds(2));
}

BOOST_AUTO_TEST_CASE( testMainLoop ){
  targetVoltage->set(38);
    std::cout << "TEST: to device sync" << std::endl;
  controlSystemPVManager->synchronize( toDeviceProcessVariables, emptyPVList );
  //boost::this_thread::sleep(boost::posix_time::seconds(2));
   std::cout << "TEST: from device sync" << std::endl;
  controlSystemPVManager->synchronize( emptyPVList, fromDeviceProcessVariables );

  BOOST_CHECK( *monitorVoltage == 38 ); 
}

BOOST_AUTO_TEST_SUITE_END()
