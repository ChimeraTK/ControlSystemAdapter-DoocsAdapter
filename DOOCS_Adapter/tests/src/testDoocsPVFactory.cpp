// Define a name for the test module.
#define BOOST_TEST_MODULE DoocsPVFactoryTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include <sstream>
#include <typeinfo>

#include "DoocsPVFactory.h"
#include "DoocsProcessScalar.h"
#include <ControlSystemAdapter/ControlSystemPVManager.h>
#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/ProcessScalar.h>

#include "emptyServerFunctions.h"

using namespace boost::unit_test_framework;
using namespace mtca4u;
using boost::shared_ptr;

template<class T, class DOOCS_T, class DOOCS_VALUE_T>
static void testCreateProcessScalar(typename ProcessVariable::SharedPtr processVariable,
				    DoocsPVFactory & factory){
  // have the variable created and check that it is the right type
  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create( processVariable );
  // get the raw pointer and dynamic cast it to the expected type
  DoocsProcessScalar<T, DOOCS_T, DOOCS_VALUE_T> * doocsScalarType = 
    dynamic_cast< DoocsProcessScalar<T, DOOCS_T, DOOCS_VALUE_T> * > (doocsVariableAsDFct.get());
  // if the cast succeeds the factory works as expected
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

// After you finished all test you have to end the test suite.
BOOST_AUTO_TEST_SUITE_END()

