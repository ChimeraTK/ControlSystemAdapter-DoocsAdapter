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

  boost::shared_ptr< ProcessArray<T> > deviceVariable =
    devManager->createProcessArray<T>(controlSystemToDevice,"toDeviceVariable", 1);
  boost::shared_ptr< ProcessArray<T> > controlSystemVariable = 
    csManager->getProcessArray<T>("toDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0)=0;
  controlSystemVariable->accessData(0)=0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<T, D_int, int> doocsScalar( NULL, controlSystemVariable, syncUtil );

  doocsScalar=42;
  BOOST_CHECK( controlSystemVariable->accessData(0) == 42 );

  // receive on the device side and check that the value has arrived
  deviceVariable->readNonBlocking();
  BOOST_CHECK( deviceVariable->accessData(0) == 42 );

  // check with negative values, and cast so unsigned gets correct results

  // check that the set_value overloading is working by calling the function of the base class
  // (note: cast to a reference, otherwise inheritance/ virtual functions calls do not work)
  static_cast<D_int&>(doocsScalar).set_value(-13);
  BOOST_CHECK( controlSystemVariable->accessData(0) == static_cast<T>(-13) );

  // receive on the device side and check that the value has arrived
  deviceVariable->readNonBlocking();
  BOOST_CHECK( deviceVariable->accessData(0) == static_cast<T>(-13) );
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

  boost::shared_ptr< ProcessArray<float> > deviceFloat =
    devManager->createProcessArray<float>(controlSystemToDevice,"toDeviceFloat",1);
  boost::shared_ptr< ProcessArray<float> > controlSystemFloat = 
    csManager->getProcessArray<float>("toDeviceFloat");
  // set the variables to 0
  deviceFloat->accessData(0)=0;
  controlSystemFloat->accessData(0)=0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<float, D_float, float> doocsScalar( NULL, controlSystemFloat, syncUtil );

  doocsScalar=12.125;
  BOOST_CHECK( controlSystemFloat->accessData(0) == 12.125 );

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK( deviceFloat->accessData(0) == 12.125 );

  // check that the set_value overloading is working by calling the function of the base class
  // (note: cast to a reference, otherwise inheritance/ virtual functions calls do not work)
  static_cast<D_float&>(doocsScalar).set_value(-13.);
  BOOST_CHECK( controlSystemFloat->accessData(0) == -13. );

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK( deviceFloat->accessData(0) == -13. );
}

BOOST_AUTO_TEST_CASE(toDeviceDoubleTest){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  boost::shared_ptr< ProcessArray<double> > deviceDouble =
    devManager->createProcessArray<double>(controlSystemToDevice,"toDeviceDouble",1);
  boost::shared_ptr< ProcessArray<double> > controlSystemDouble = 
    csManager->getProcessArray<double>("toDeviceDouble");
  // set the variables to 0
  deviceDouble->accessData(0)=0;
  controlSystemDouble->accessData(0)=0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<double, D_double, double> doocsScalar( NULL, controlSystemDouble, syncUtil );

  doocsScalar=12.125;
  BOOST_CHECK( controlSystemDouble->accessData(0) == 12.125 );

  // receive on the device side and check that the value has arrived
  deviceDouble->readNonBlocking();
  BOOST_CHECK( deviceDouble->accessData(0) == 12.125 );

  // check that the set_value overloading is working by calling the function of the base class
  // (note: cast to a reference, otherwise inheritance/ virtual functions calls do not work)
  static_cast<D_double&>(doocsScalar).set_value(-13.);
  BOOST_CHECK( controlSystemDouble->accessData(0) == -13. );

  // receive on the device side and check that the value has arrived
  deviceDouble->readNonBlocking();
  BOOST_CHECK( deviceDouble->accessData(0) == -13. );
}

BOOST_AUTO_TEST_CASE_TEMPLATE( fromDeviceIntegerTypeTest, T, integer_test_types ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  typename ProcessArray<T>::SharedPtr deviceVariable =
    devManager->createProcessArray<T>(deviceToControlSystem,"fromDeviceVariable",1);
  typename ProcessArray<T>::SharedPtr controlSystemVariable = 
    csManager->getProcessArray<T>("fromDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0)=0;
  controlSystemVariable->accessData(0)=0;

  // initialise the doocs scalar
  DoocsProcessScalar<T, D_int, int> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar=0;

  deviceVariable->accessData(0)=42;
  deviceVariable->write();

  BOOST_CHECK( controlSystemVariable->accessData(0) == 0 );
  BOOST_CHECK( doocsScalar.value() == 0 );

  syncUtil.receiveAll();
  BOOST_CHECK( controlSystemVariable->accessData(0) == 42 );
  BOOST_CHECK( doocsScalar.value() == 42 );

  // negative test for signed int, with cast for uints
  deviceVariable->accessData(0)=-13;
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

  ProcessArray<float>::SharedPtr deviceVariable =
    devManager->createProcessArray<float>(deviceToControlSystem,"fromDeviceVariable",1);
  ProcessArray<float>::SharedPtr controlSystemVariable = 
    csManager->getProcessArray<float>("fromDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0)=0;
  controlSystemVariable->accessData(0)=0;

  // initialise the doocs scalar
  DoocsProcessScalar<float, D_float, float> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar=0;

  deviceVariable->accessData(0)=12.125;
  deviceVariable->write();

  BOOST_CHECK( controlSystemVariable->accessData(0) == 0 );
  BOOST_CHECK( doocsScalar.value() == 0 );

  syncUtil.receiveAll();
  BOOST_CHECK( controlSystemVariable->accessData(0) == 12.125 );
  BOOST_CHECK( doocsScalar.value() == 12.125 );
}

BOOST_AUTO_TEST_CASE( fromDeviceDoubleTest ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  ProcessArray<double>::SharedPtr deviceVariable =
    devManager->createProcessArray<double>(deviceToControlSystem,"fromDeviceVariable",1);
  ProcessArray<double>::SharedPtr controlSystemVariable = 
    csManager->getProcessArray<double>("fromDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0)=0;
  controlSystemVariable->accessData(0)=0;

  // initialise the doocs scalar
  DoocsProcessScalar<double, D_double, double> doocsScalar( NULL, controlSystemVariable, syncUtil );
  doocsScalar=0;

  deviceVariable->accessData(0)=12.125;
  deviceVariable->write();

  BOOST_CHECK( controlSystemVariable->accessData(0) == 0 );
  BOOST_CHECK( doocsScalar.value() == 0 );

  syncUtil.receiveAll();
  BOOST_CHECK( controlSystemVariable->accessData(0) == 12.125 );
  BOOST_CHECK( doocsScalar.value() == 12.125 );
}

BOOST_AUTO_TEST_SUITE_END()
