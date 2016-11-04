#define BOOST_TEST_MODULE DoocsProcessScalarTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "DoocsProcessScalar.h"
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <d_fct.h>

#include "emptyServerFunctions.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

// use boost meta-programming to use test case templates
// The list of types is an mpl type
typedef boost::mpl::list<int32_t, uint32_t,
			 int16_t, uint16_t,
			 int8_t, uint8_t> integer_test_types;

BOOST_AUTO_TEST_SUITE( DoocsProcessScalarTestSuite )

BOOST_AUTO_TEST_CASE_TEMPLATE( toDeviceIntegerTypeTest, T, integer_test_types ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  boost::shared_ptr< ProcessScalar< T> > deviceVariable =
    devManager->createProcessScalar<T>(controlSystemToDevice,"toDeviceVariable");
  boost::shared_ptr< ProcessScalar<T> > controlSystemVariable = 
    csManager->getProcessScalar<T>("toDeviceVariable");
  // set the variables to 0
  *deviceVariable=0;
  *controlSystemVariable=0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<T, D_int, int> doocsScalar( NULL, controlSystemVariable, syncUtil );

  doocsScalar=42;
  BOOST_CHECK( *controlSystemVariable == 42 );

  // receive on the device side and check that the value has arrived
  deviceVariable->readNonBlocking();
  BOOST_CHECK( *deviceVariable == 42 );

  // check with negative values, and cast so unsigned gets correct results

  // check that the set_value overloading is working by calling the function of the base class
  // (note: cast to a reference, otherwise inheritance/ virtual functions calls do not work)
  static_cast<D_int&>(doocsScalar).set_value(-13);
  BOOST_CHECK( *controlSystemVariable == static_cast<T>(-13) );

  // receive on the device side and check that the value has arrived
  deviceVariable->readNonBlocking();
  BOOST_CHECK( *deviceVariable == static_cast<T>(-13) );
}

// as boost testing does not allow multiple template parameters we use code
// duplication to do DoocsProcessScalar<float, D_float, float> and
// DoocsProcessScalar<double, D_double, double>
BOOST_AUTO_TEST_CASE(toDeviceFloatTest){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  boost::shared_ptr< ProcessScalar<float> > deviceFloat =
    devManager->createProcessScalar<float>(controlSystemToDevice,"toDeviceFloat");
  boost::shared_ptr< ProcessScalar<float> > controlSystemFloat = 
    csManager->getProcessScalar<float>("toDeviceFloat");
  // set the variables to 0
  *deviceFloat=0;
  *controlSystemFloat=0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<float, D_float, float> doocsScalar( NULL, controlSystemFloat, syncUtil );

  doocsScalar=12.125;
  BOOST_CHECK( *controlSystemFloat == 12.125 );

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK( *deviceFloat == 12.125 );

  // check that the set_value overloading is working by calling the function of the base class
  // (note: cast to a reference, otherwise inheritance/ virtual functions calls do not work)
  static_cast<D_float&>(doocsScalar).set_value(-13.);
  BOOST_CHECK( *controlSystemFloat == -13. );

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK( *deviceFloat == -13. );
}

BOOST_AUTO_TEST_CASE(toDeviceDoubleTest){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  boost::shared_ptr< ProcessScalar<double> > deviceDouble =
    devManager->createProcessScalar<double>(controlSystemToDevice,"toDeviceDouble");
  boost::shared_ptr< ProcessScalar<double> > controlSystemDouble = 
    csManager->getProcessScalar<double>("toDeviceDouble");
  // set the variables to 0
  *deviceDouble=0;
  *controlSystemDouble=0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<double, D_double, double> doocsScalar( NULL, controlSystemDouble, syncUtil );

  doocsScalar=12.125;
  BOOST_CHECK( *controlSystemDouble == 12.125 );

  // receive on the device side and check that the value has arrived
  deviceDouble->readNonBlocking();
  BOOST_CHECK( *deviceDouble == 12.125 );

  // check that the set_value overloading is working by calling the function of the base class
  // (note: cast to a reference, otherwise inheritance/ virtual functions calls do not work)
  static_cast<D_double&>(doocsScalar).set_value(-13.);
  BOOST_CHECK( *controlSystemDouble == -13. );

  // receive on the device side and check that the value has arrived
  deviceDouble->readNonBlocking();
  BOOST_CHECK( *deviceDouble == -13. );
}

BOOST_AUTO_TEST_CASE_TEMPLATE( fromDeviceIntegerTypeTest, T, integer_test_types ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  typename ProcessScalar<T>::SharedPtr deviceVariable =
    devManager->createProcessScalar<T>(deviceToControlSystem,"fromDeviceVariable");
  typename ProcessScalar<T>::SharedPtr controlSystemVariable = 
    csManager->getProcessScalar<T>("fromDeviceVariable");
  // set the variables to 0
  *deviceVariable=0;
  *controlSystemVariable=0;

  // initialise the doocs scalar
  DoocsProcessScalar<T, D_int, int> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar=0;

  *deviceVariable=42;
  deviceVariable->write();

  BOOST_CHECK( *controlSystemVariable == 0 );
  BOOST_CHECK( doocsScalar.value() == 0 );

  syncUtil.receiveAll();
  BOOST_CHECK( *controlSystemVariable == 42 );
  BOOST_CHECK( doocsScalar.value() == 42 );

  // negative test for signed int, with cast for uints
  *deviceVariable=-13;
  deviceVariable->write();
  syncUtil.receiveAll();
  BOOST_CHECK( doocsScalar.value() == static_cast<int>(static_cast<T>(-13)) );
}

BOOST_AUTO_TEST_CASE( fromDeviceFloatTest ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  ProcessScalar<float>::SharedPtr deviceVariable =
    devManager->createProcessScalar<float>(deviceToControlSystem,"fromDeviceVariable");
  ProcessScalar<float>::SharedPtr controlSystemVariable = 
    csManager->getProcessScalar<float>("fromDeviceVariable");
  // set the variables to 0
  *deviceVariable=0;
  *controlSystemVariable=0;

  // initialise the doocs scalar
  DoocsProcessScalar<float, D_float, float> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar=0;

  *deviceVariable=12.125;
  deviceVariable->write();

  BOOST_CHECK( *controlSystemVariable == 0 );
  BOOST_CHECK( doocsScalar.value() == 0 );

  syncUtil.receiveAll();
  BOOST_CHECK( *controlSystemVariable == 12.125 );
  BOOST_CHECK( doocsScalar.value() == 12.125 );
}

BOOST_AUTO_TEST_CASE( fromDeviceDoubleTest ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  ProcessScalar<double>::SharedPtr deviceVariable =
    devManager->createProcessScalar<double>(deviceToControlSystem,"fromDeviceVariable");
  ProcessScalar<double>::SharedPtr controlSystemVariable = 
    csManager->getProcessScalar<double>("fromDeviceVariable");
  // set the variables to 0
  *deviceVariable=0;
  *controlSystemVariable=0;

  // initialise the doocs scalar
  DoocsProcessScalar<double, D_double, double> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar=0;

  *deviceVariable=12.125;
  deviceVariable->write();

  BOOST_CHECK( *controlSystemVariable == 0 );
  BOOST_CHECK( doocsScalar.value() == 0 );

  syncUtil.receiveAll();
  BOOST_CHECK( *controlSystemVariable == 12.125 );
  BOOST_CHECK( doocsScalar.value() == 12.125 );
}

BOOST_AUTO_TEST_SUITE_END()
