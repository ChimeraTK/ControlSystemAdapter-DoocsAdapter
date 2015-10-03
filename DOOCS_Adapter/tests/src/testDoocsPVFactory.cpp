// Define a name for the test module.
#define BOOST_TEST_MODULE DoocsPVFactoryTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <sstream>
#include <typeinfo>

#include "DoocsPVFactory.h"
#include "DoocsProcessScalar.h"
#include "DoocsProcessArray.h"
#include <ControlSystemAdapter/ControlSystemPVManager.h>
#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/ProcessScalar.h>

#include "emptyServerFunctions.h"

using namespace boost::unit_test_framework;
using namespace mtca4u;
using boost::shared_ptr;

// use boost meta-programming to use test case templates
// The list of types is an mpl type
typedef boost::mpl::list<int32_t, uint32_t,
			 int16_t, uint16_t,
			 int8_t, uint8_t,
			 float, double> simple_test_types;

// class which exposes the protected member functions for testing
class TestableDoocsPVFactory: public DoocsPVFactory{
public:
  TestableDoocsPVFactory(EqFct * const eqFct,
    boost::shared_ptr<ControlSystemSynchronizationUtility> const & syncUtility)
  : DoocsPVFactory(eqFct, syncUtility){
  }

  template<class T, class DOOCS_T, class DOOCS_VALUE_T>
  typename boost::shared_ptr<D_fct> createDoocsScalar(typename ProcessVariable::SharedPtr & processVariable){
    return DoocsPVFactory::createDoocsScalar<T, DOOCS_T, DOOCS_VALUE_T>(processVariable);
  }

  template<class T>
  typename boost::shared_ptr<D_fct> createDoocsArray(typename ProcessVariable::SharedPtr & processVariable){
     return DoocsPVFactory::createDoocsArray<T>(processVariable);   
  }
};

template<class T, class DOOCS_T, class DOOCS_VALUE_T>
static void testCreateProcessScalar(typename ProcessVariable::SharedPtr processVariable,
				    DoocsPVFactory & factory){
  // have the variable created and check that it is the right type
  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create( processVariable );
  // get the raw pointer and dynamic cast it to the expected type
  DoocsProcessScalar<T, DOOCS_T, DOOCS_VALUE_T> * doocsScalarType = 
    dynamic_cast< DoocsProcessScalar<T, DOOCS_T, DOOCS_VALUE_T> * > (doocsVariableAsDFct.get());
  // if the cast succeeds the factory works as expected we are done
  std::stringstream errorMessage;
  errorMessage << "testCreateProcessScalar failed for type " << typeid(T).name();
  BOOST_CHECK_MESSAGE(doocsScalarType, errorMessage.str());
}

BOOST_AUTO_TEST_SUITE( PVManagerTestSuite )

BOOST_AUTO_TEST_CASE( testCreateScalars ) {
  std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  // create all process variables before creating the sync util
  devManager->createProcessScalarControlSystemToDevice<int32_t>("int32");
  devManager->createProcessScalarControlSystemToDevice<uint32_t>("uint32");
  devManager->createProcessScalarControlSystemToDevice<int16_t>("int16");
  devManager->createProcessScalarControlSystemToDevice<uint16_t>("uint16");
  devManager->createProcessScalarControlSystemToDevice<int8_t>("int8");
  devManager->createProcessScalarControlSystemToDevice<uint8_t>("uint8");
  devManager->createProcessScalarControlSystemToDevice<float>("float");
  devManager->createProcessScalarControlSystemToDevice<double>("double");

  shared_ptr<ControlSystemSynchronizationUtility> syncUtil(
    new ControlSystemSynchronizationUtility(csManager));

  DoocsPVFactory factory(NULL /*eqFct*/, syncUtil);

  // We insert check points with integers so we know where the algorithm kicks out in case of an error.
  // These checkpoints are always true.
  testCreateProcessScalar<int32_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<int32_t>("int32")),
    factory);
  BOOST_CHECK(-32);
  testCreateProcessScalar<uint32_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<uint32_t>("uint32")),
    factory);
  BOOST_CHECK(32);
  testCreateProcessScalar<int16_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<int16_t>("int16")),
    factory);
  BOOST_CHECK(-16);
  testCreateProcessScalar<uint16_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<uint16_t>("uint16")),
    factory);
  BOOST_CHECK(16);
  testCreateProcessScalar<int8_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<int8_t>("int8")),
    factory);
  BOOST_CHECK(-8);
  testCreateProcessScalar<uint8_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<uint8_t>("uint8")),
    factory);
  BOOST_CHECK(8);
  testCreateProcessScalar<float, D_float, float>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<float>("float")),
    factory);
  BOOST_CHECK(0.5);
  testCreateProcessScalar<double, D_double, double>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<double>("double")),
    factory);
  
}

BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateArray, T, simple_test_types ){
  std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 10;
  devManager->createProcessArrayControlSystemToDevice<T>("toDeviceArray",arraySize);

  shared_ptr<ControlSystemSynchronizationUtility> syncUtil(
    new ControlSystemSynchronizationUtility(csManager));
  DoocsPVFactory factory(NULL /*eqFct*/, syncUtil);

  // have the variable created and check that it is the right type
  ProcessVariable::SharedPtr processVariable = 
    csManager->getProcessArray<T>("toDeviceArray");
  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create(processVariable);

  // get the raw pointer and dynamic cast it to the expected type
  DoocsProcessArray<T> * doocsArray = 
    dynamic_cast< DoocsProcessArray<T> * > (doocsVariableAsDFct.get());

  // if the cast succeeds the factory works as expected we are done
  BOOST_REQUIRE(doocsArray);
  BOOST_CHECK( static_cast<size_t>(doocsArray->max_length()) == arraySize );
}

BOOST_AUTO_TEST_CASE( testErrorHandling ){
    std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 10;
  // int64 is not supported yet
  devManager->createProcessArrayControlSystemToDevice<int64_t>("toDeviceArray",arraySize);
  devManager->createProcessScalarControlSystemToDevice<int64_t>("toDeviceInt");

  shared_ptr<ControlSystemSynchronizationUtility> syncUtil(
    new ControlSystemSynchronizationUtility(csManager));
  TestableDoocsPVFactory testableFactory(NULL /*eqFct*/, syncUtil);

  ProcessVariable::SharedPtr processVariable = 
    csManager->getProcessScalar<int64_t>("toDeviceInt");
  // Intentionally put the int64 scalar to the int32 create function.
  // Unfortunately BOOST_CHECK cannot deal with multiple template parameters,
  // so we have to trick it
  try{
    testableFactory.createDoocsScalar<int32_t, D_int, int>( processVariable );
    BOOST_FAIL( "createDoocsScalar did not throw as expected");
  }catch(std::invalid_argument &){
  }

  // now the same with arrays
   processVariable = csManager->getProcessArray<int64_t>("toDeviceArray");
   BOOST_REQUIRE(processVariable);
   BOOST_CHECK_THROW( testableFactory.createDoocsArray<int32_t>(processVariable),
		      std::invalid_argument );
}

// After you finished all test you have to end the test suite.
BOOST_AUTO_TEST_SUITE_END()

