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
#include "DoocsSpectrum.h"
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>

#include "emptyServerFunctions.h"
#include "getAllVariableNames.h"
#include "PropertyDescription.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;
using boost::shared_ptr;

// use boost meta-programming to use test case templates
// The list of types is an mpl type
typedef boost::mpl::list<int32_t, uint32_t,
			 int16_t, uint16_t,
			 int8_t, uint8_t,
			 float, double> simple_test_types;

EqFct myEqFct("MY_EQ_FCT");

template<class DOOCS_PRIMITIVE_T, class DOOCS_T>
static void testCreateProcessScalar(std::shared_ptr<PropertyDescription> const & propertyDescription,
				    DoocsPVFactory & factory, std::string const & expectedPropertyName){
  
  // have the variable created and check that it is the right type
  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.new_create( propertyDescription );
  // get the raw pointer and dynamic cast it to the expected type
  DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T> * doocsScalarType = 
    dynamic_cast< DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T> * > (doocsVariableAsDFct.get());
  // if the cast succeeds the factory works as expected we are done
  std::string errorMessage = std::string("testCreateProcessScalar failed for type ") + typeid(DOOCS_PRIMITIVE_T).name();
  BOOST_CHECK_MESSAGE(doocsScalarType, errorMessage);
  errorMessage = std::string("Error checking property name: expectedPropertyName '")
    + expectedPropertyName + "', property_name() '" + doocsVariableAsDFct->property_name() + "'";
  BOOST_CHECK_MESSAGE( expectedPropertyName == doocsVariableAsDFct->property_name() , errorMessage);
}

BOOST_AUTO_TEST_SUITE( PVManagerTestSuite )

BOOST_AUTO_TEST_CASE( testAutoCreateScalars ) {
  std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  devManager->createProcessArray<int32_t>(controlSystemToDevice,"/I/int32",1);
  devManager->createProcessArray<uint32_t>(controlSystemToDevice,"/U/uint32",1);
  devManager->createProcessArray<int16_t>(controlSystemToDevice,"/I/int16",1);
  devManager->createProcessArray<uint16_t>(controlSystemToDevice,"/U/uint16",1);
  devManager->createProcessArray<int8_t>(controlSystemToDevice,"/I/int8",1);
  devManager->createProcessArray<uint8_t>(controlSystemToDevice,"/U/uint8",1);
  devManager->createProcessArray<float>(controlSystemToDevice,"/FP/float",1);
  devManager->createProcessArray<double>(controlSystemToDevice,"/FP/double",1);

  DoocsUpdater updater;
  
  DoocsPVFactory factory(&myEqFct, updater, csManager);

  // We insert check points with integers so we know where the algorithm kicks out in case of an error.
  // These checkpoints are always true.
  testCreateProcessScalar<int32_t, D_int>(
    std::make_shared<AutoPropertyDescription>("I/int32", "I", "int32"), factory, "int32 ");// DOOCS property names always have a space (and potentially some description)"
  BOOST_CHECK(-32);
  testCreateProcessScalar<int32_t, D_int>(
    std::make_shared<AutoPropertyDescription>("U/uint32", "I", "uint32"), factory, "uint32 ");
  BOOST_CHECK(32);
  testCreateProcessScalar<int32_t, D_int>(
    std::make_shared<AutoPropertyDescription>("I/int16", "I", "int16"), factory, "int16 ");// DOOCS property names always have a space (and potentially some description)"
  BOOST_CHECK(-16);
  testCreateProcessScalar<int32_t, D_int>(
    std::make_shared<AutoPropertyDescription>("U/uint16", "I", "uint16"), factory, "uint16 ");// DOOCS property names always have a space (and potentially some description)"
  BOOST_CHECK(16);
  testCreateProcessScalar<int32_t, D_int>(
    std::make_shared<AutoPropertyDescription>("I/int8", "I", "int8"), factory, "int8 ");// DOOCS property names always have a space (and potentially some description)"
  BOOST_CHECK(-8);
  testCreateProcessScalar<int32_t, D_int>(
    std::make_shared<AutoPropertyDescription>("U/uint8", "I", "uint8"), factory, "uint8 ");// DOOCS property names always have a space (and potentially some description)"
  BOOST_CHECK(8);
  testCreateProcessScalar<float, D_float>(
    std::make_shared<AutoPropertyDescription>("FP/float", "FP", "float"), factory, "float ");// DOOCS property names always have a space (and potentially some description)"
  BOOST_CHECK(0.5);
  testCreateProcessScalar<double, D_double>(
    std::make_shared<AutoPropertyDescription>("FP/double", "FP", "double"), factory, "double ");// DOOCS property names always have a space (and potentially some description)"
  BOOST_CHECK(32);
}

//BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateArray, T, simple_test_types ){
//  std::pair< shared_ptr<ControlSystemPVManager>,
//	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
//  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
//  shared_ptr<DevicePVManager> devManager = pvManagers.second;
//
//  static const size_t arraySize = 10;
//  devManager->createProcessArray<T>(controlSystemToDevice,"A/toDeviceArray",arraySize);
//
//  DoocsPVFactory factory(&myEqFct);
//
//  // have the variable created and check that it is the right type
//  ProcessVariable::SharedPtr processVariable = 
//    csManager->getProcessArray<T>("A/toDeviceArray");
//  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create(processVariable);
//
//  // get the raw pointer and dynamic cast it to the expected type
//  DoocsProcessArray<T> * doocsArray = 
//    dynamic_cast< DoocsProcessArray<T> * > (doocsVariableAsDFct.get());
//
//  // if the cast succeeds the factory works as expected we are done
//  BOOST_REQUIRE(doocsArray);
//  BOOST_CHECK( static_cast<size_t>(doocsArray->max_length()) == arraySize );
//}

BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateSpectrum, T, simple_test_types ){
  std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 10;
  // array 1 will have default settings, array 2 custom start and increment
  devManager->createProcessArray<T>(controlSystemToDevice,"A/fromDeviceArray1",arraySize);
  devManager->createProcessArray<T>(controlSystemToDevice,"A/fromDeviceArray2",arraySize);

  // we need this later anyway, do we make a temporary variable
  auto pvNames = ChimeraTK::getAllVariableNames( csManager );
  
  DoocsUpdater updater;
  
  DoocsPVFactory factory(&myEqFct, updater, csManager);

  auto propertyDescriptions = { std::make_shared<AutoPropertyDescription>("A/fromDeviceArray1","A","fromDeviceArray1"), std::make_shared<AutoPropertyDescription>("A/fromDeviceArray2","A","fromDeviceArray2")};
  
  // have the variable created and check that it is the right type
  for (auto const & description : propertyDescriptions){
    boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.new_create(description);

    // get the raw pointer and dynamic cast it to the expected type
    DoocsSpectrum * doocsSpectrum = 
      dynamic_cast< DoocsSpectrum * > (doocsVariableAsDFct.get());

    // if the cast succeeds the factory works as expected we are done
    BOOST_REQUIRE(doocsSpectrum);
    BOOST_CHECK( static_cast<size_t>(doocsSpectrum->max_length()) == arraySize );
  }
  // FIXME: add tests for x-axis config
}

BOOST_AUTO_TEST_CASE( testErrorHandling ){
    std::pair< shared_ptr<ControlSystemPVManager>,
	     shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  // int64 is not supported yet
  devManager->createProcessArray<int64_t>(controlSystemToDevice,"I/toDeviceInt",1);

  DoocsUpdater updater;
  
  DoocsPVFactory factory(&myEqFct, updater, csManager);

  ProcessVariable::SharedPtr processScalar = 
    csManager->getProcessArray<int64_t>("I/toDeviceInt");
  // Intentionally put the int64 scalar to the int32 create function.
  // Unfortunately BOOST_CHECK cannot deal with multiple template parameters,
  // so we have to trick it
  auto description = std::make_shared<AutoPropertyDescription>("I/toDeviceInt", "I", "toDeviceInt");
  try{    factory.new_create( description );
    // In a working unit test this line should not be hit, so er exclude it
    // from the coverage report.
    BOOST_ERROR( "createDoocsScalar did not throw as expected");//LCOV_EXCL_LINE
  }catch(std::invalid_argument &e){
    BOOST_CHECK( std::string("unsupported value type") == e.what() );
  }
}

// After you finished all test you have to end the test suite.
BOOST_AUTO_TEST_SUITE_END()

