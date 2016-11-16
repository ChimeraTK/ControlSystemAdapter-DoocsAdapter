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
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessArray.h>

#include "emptyServerFunctions.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;
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
    return DoocsPVFactory::createDoocsProperty<T, DOOCS_T, DOOCS_VALUE_T>(processVariable);
  }

  template<class T, class DOOCS_T, class DOOCS_VALUE_T>
  typename boost::shared_ptr<D_fct> createDoocsArray(typename ProcessVariable::SharedPtr & processVariable){
    return DoocsPVFactory::createDoocsProperty<T, DOOCS_T, DOOCS_VALUE_T>(processVariable);   
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
  devManager->createProcessArray<int32_t>(controlSystemToDevice,"int32",1);
  devManager->createProcessArray<uint32_t>(controlSystemToDevice,"uint32",1);
  devManager->createProcessArray<int16_t>(controlSystemToDevice,"int16",1);
  devManager->createProcessArray<uint16_t>(controlSystemToDevice,"uint16",1);
  devManager->createProcessArray<int8_t>(controlSystemToDevice,"int8",1);
  devManager->createProcessArray<uint8_t>(controlSystemToDevice,"uint8",1);
  devManager->createProcessArray<float>(controlSystemToDevice,"float",1);
  devManager->createProcessArray<double>(controlSystemToDevice,"double",1);

  shared_ptr<ControlSystemSynchronizationUtility> syncUtil(
    new ControlSystemSynchronizationUtility(csManager));

  DoocsPVFactory factory(NULL /*eqFct*/, syncUtil);

  // We insert check points with integers so we know where the algorithm kicks out in case of an error.
  // These checkpoints are always true.
  testCreateProcessScalar<int32_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<int32_t>("int32")),
    factory);
  BOOST_CHECK(-32);
  testCreateProcessScalar<uint32_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<uint32_t>("uint32")),
    factory);
  BOOST_CHECK(32);
  testCreateProcessScalar<int16_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<int16_t>("int16")),
    factory);
  BOOST_CHECK(-16);
  testCreateProcessScalar<uint16_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<uint16_t>("uint16")),
    factory);
  BOOST_CHECK(16);
  testCreateProcessScalar<int8_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<int8_t>("int8")),
    factory);
  BOOST_CHECK(-8);
  testCreateProcessScalar<uint8_t, D_int, int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<uint8_t>("uint8")),
    factory);
  BOOST_CHECK(8);
  testCreateProcessScalar<float, D_float, float>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<float>("float")),
    factory);
  BOOST_CHECK(0.5);
  testCreateProcessScalar<double, D_double, double>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessArray<double>("double")),
    factory);
  
}

BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateArray, T, simple_test_types ){
  std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 10;
  devManager->createProcessArray<T>(controlSystemToDevice,"toDeviceArray",arraySize);

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
  devManager->createProcessArray<int64_t>(controlSystemToDevice,"toDeviceArray",arraySize);
  devManager->createProcessArray<int64_t>(controlSystemToDevice,"toDeviceInt",1);

  shared_ptr<ControlSystemSynchronizationUtility> syncUtil(
    new ControlSystemSynchronizationUtility(csManager));
  TestableDoocsPVFactory testableFactory(NULL /*eqFct*/, syncUtil);

  ProcessVariable::SharedPtr processScalar = 
    csManager->getProcessArray<int64_t>("toDeviceInt");
  // Intentionally put the int64 scalar to the int32 create function.
  // Unfortunately BOOST_CHECK cannot deal with multiple template parameters,
  // so we have to trick it
  try{
    testableFactory.createDoocsScalar<int32_t, D_int, int>( processScalar );
    // In a working unit test this line should not be hit, so er exclude it
    // from the coverage report.
    BOOST_FAIL( "createDoocsScalar did not throw as expected");//LCOV_EXCL_LINE
  }catch(std::invalid_argument &){
  }

  // now the same with arrays
  ProcessVariable::SharedPtr processArray = csManager->getProcessArray<int64_t>("toDeviceArray");
  BOOST_CHECK_THROW( ( testableFactory.createDoocsArray<int32_t, D_int, int>(processArray) ),
		      std::invalid_argument );

   // finally we check that the create method catches the not-supported type.
  try{
    testableFactory.create( processScalar );
    BOOST_FAIL( "create did not throw as expected");//LCOV_EXCL_LINE
  }catch(std::invalid_argument &e){
    BOOST_CHECK( std::string("unsupported value type") == e.what() );
  }
  
  // and the same for the scalar, just to cover all cases
  try{
    testableFactory.create( processArray );
    BOOST_FAIL( "create did not throw as expected");//LCOV_EXCL_LINE
  }catch(std::invalid_argument &e){
    BOOST_CHECK( std::string("unsupported value type") == e.what() );
  }
   
}

// After you finished all test you have to end the test suite.
BOOST_AUTO_TEST_SUITE_END()

