// Define a name for the test module.
#define BOOST_TEST_MODULE DoocsPVFactoryTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include "DoocsPVFactory.h"
#include "DoocsProcessScalar.h"
#include <ControlSystemAdapter/ControlSystemPVManager.h>
#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/ProcessScalar.h>

#include "emptyServerFunctions.h"

using namespace boost::unit_test_framework;
using namespace mtca4u;
using boost::shared_ptr;

template<class T, class DOOCS_T> static void testCreateProcessScalar(typename ProcessVariable::SharedPtr processVariable,
								     DoocsPVFactory & factory){
  // have the variable created and check that it is the right type
  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create( processVariable );
  // get the raw pointer and dynamic cast it to the expected type
  DoocsProcessScalar<T, DOOCS_T> * doocsScalarType = 
    dynamic_cast< DoocsProcessScalar<T, DOOCS_T> * > (doocsVariableAsDFct.get());
  // if the cast succeeds the factory works as expected
  BOOST_CHECK(doocsScalarType);
}

BOOST_AUTO_TEST_SUITE( PVManagerTestSuite )

BOOST_AUTO_TEST_CASE( testCreateScalars ) {
  std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  // create all process variables before creating the sync util
  devManager->createProcessScalarControlSystemToDevice<int32_t>("int32");
  ProcessVariable::SharedPtr sv( csManager->getProcessScalar<int32_t>("int32") );

  shared_ptr<ControlSystemSynchronizationUtility> syncUtil(
    new ControlSystemSynchronizationUtility(csManager));

  DoocsPVFactory factory(NULL /*eqFct*/, syncUtil);

  testCreateProcessScalar<int32_t, D_int>(
    boost::dynamic_pointer_cast<ProcessVariable>(csManager->getProcessScalar<int32_t>("int32")),
    factory);
  
}

// After you finished all test you have to end the test suite.
BOOST_AUTO_TEST_SUITE_END()

