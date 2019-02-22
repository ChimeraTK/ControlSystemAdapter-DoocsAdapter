// Define a name for the test module.
#define BOOST_TEST_MODULE DoocsPVFactoryTest
// Only after defining the name include the unit test header.
#include <boost/mpl/list.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>

#include <sstream>
#include <typeinfo>

#include "DoocsPVFactory.h"
#include "DoocsProcessArray.h"
#include "DoocsProcessScalar.h"
#include "DoocsSpectrum.h"
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>

#include "PropertyDescription.h"
#include "emptyServerFunctions.h"
#include "getAllVariableNames.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;
using boost::shared_ptr;

// use boost meta-programming to use test case templates
// The list of types is an mpl type
typedef boost::mpl::list<int64_t, uint64_t, int32_t, uint32_t, int16_t, uint16_t, int8_t, uint8_t, float, double>
    simple_test_types;

EqFct myEqFct("MY_EQ_FCT");

template<class DOOCS_PRIMITIVE_T, class DOOCS_T>
static void testCreateProcessScalar(std::shared_ptr<PropertyDescription> const& propertyDescription,
    DoocsPVFactory& factory, std::string const& expectedPropertyName) {
  // have the variable created and check that it is the right type
  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create(propertyDescription);
  // get the raw pointer and dynamic cast it to the expected type
  DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>* doocsScalarType =
      dynamic_cast<DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>*>(doocsVariableAsDFct.get());
  // if the cast succeeds the factory works as expected we are done
  std::string errorMessage = std::string("testCreateProcessScalar failed for type ") + typeid(DOOCS_PRIMITIVE_T).name();
  BOOST_CHECK_MESSAGE(doocsScalarType, errorMessage);
  errorMessage = std::string("Error checking property name: expectedPropertyName '") + expectedPropertyName +
      "', property_name() '" + doocsVariableAsDFct->property_name() + "'";
  BOOST_CHECK_MESSAGE(expectedPropertyName == doocsVariableAsDFct->property_name(), errorMessage);
}

BOOST_AUTO_TEST_SUITE(PVManagerTestSuite)

BOOST_AUTO_TEST_CASE(testAutoCreateScalars) {
  std::pair<shared_ptr<ControlSystemPVManager>, shared_ptr<DevicePVManager>> pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  devManager->createProcessArray<int32_t>(controlSystemToDevice, "/I/int32", 1);
  devManager->createProcessArray<uint32_t>(controlSystemToDevice, "/U/uint32", 1);
  devManager->createProcessArray<int16_t>(controlSystemToDevice, "/I/int16", 1);
  devManager->createProcessArray<uint16_t>(controlSystemToDevice, "/U/uint16", 1);
  devManager->createProcessArray<int8_t>(controlSystemToDevice, "/I/int8", 1);
  devManager->createProcessArray<uint8_t>(controlSystemToDevice, "/U/uint8", 1);
  devManager->createProcessArray<float>(controlSystemToDevice, "/FP/float", 1);
  devManager->createProcessArray<double>(controlSystemToDevice, "/FP/double", 1);

  DoocsUpdater updater;

  DoocsPVFactory factory(&myEqFct, updater, csManager);

  // We insert check points with integers so we know where the algorithm kicks
  // out in case of an error. These checkpoints are always true.
  testCreateProcessScalar<int32_t, D_int>(std::make_shared<AutoPropertyDescription>("I/int32", "I", "int32"), factory,
      "int32 "); // DOOCS property names always have a space (and
                 // potentially some description)"
  BOOST_CHECK(-32);
  testCreateProcessScalar<int32_t, D_int>(
      std::make_shared<AutoPropertyDescription>("U/uint32", "I", "uint32"), factory, "uint32 ");
  BOOST_CHECK(32);
  testCreateProcessScalar<int32_t, D_int>(std::make_shared<AutoPropertyDescription>("I/int16", "I", "int16"), factory,
      "int16 "); // DOOCS property names always have a space (and
                 // potentially some description)"
  BOOST_CHECK(-16);
  testCreateProcessScalar<int32_t, D_int>(std::make_shared<AutoPropertyDescription>("U/uint16", "I", "uint16"), factory,
      "uint16 "); // DOOCS property names always have a space (and
                  // potentially some description)"
  BOOST_CHECK(16);
  testCreateProcessScalar<int32_t, D_int>(std::make_shared<AutoPropertyDescription>("I/int8", "I", "int8"), factory,
      "int8 "); // DOOCS property names always have a space (and potentially
                // some description)"
  BOOST_CHECK(-8);
  testCreateProcessScalar<int32_t, D_int>(std::make_shared<AutoPropertyDescription>("U/uint8", "I", "uint8"), factory,
      "uint8 "); // DOOCS property names always have a space (and
                 // potentially some description)"
  BOOST_CHECK(8);
  testCreateProcessScalar<float, D_float>(std::make_shared<AutoPropertyDescription>("FP/float", "FP", "float"), factory,
      "float "); // DOOCS property names always have a space (and
                 // potentially some description)"
  BOOST_CHECK(0.5);
  testCreateProcessScalar<double, D_double>(std::make_shared<AutoPropertyDescription>("FP/double", "FP", "double"),
      factory, "double "); // DOOCS property names always have a space (and
                           // potentially some description)"
  BOOST_CHECK(32);
}

// BOOST_AUTO_TEST_CASE_TEMPLATE( testCreateArray, T, simple_test_types ){
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
//  boost::shared_ptr<D_fct> doocsVariableAsDFct =
//  factory.create(processVariable);
//
//  // get the raw pointer and dynamic cast it to the expected type
//  DoocsProcessArray<T> * doocsArray =
//    dynamic_cast< DoocsProcessArray<T> * > (doocsVariableAsDFct.get());
//
//  // if the cast succeeds the factory works as expected we are done
//  BOOST_REQUIRE(doocsArray);
//  BOOST_CHECK( static_cast<size_t>(doocsArray->max_length()) == arraySize );
//}

BOOST_AUTO_TEST_CASE_TEMPLATE(testCreateSpectrum, T, simple_test_types) {
  std::pair<shared_ptr<ControlSystemPVManager>, shared_ptr<DevicePVManager>> pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 10;
  // array 1 will have default settings, array 2 custom start and increment
  devManager->createProcessArray<T>(controlSystemToDevice, "A/fromDeviceArray1", arraySize);
  devManager->createProcessArray<T>(controlSystemToDevice, "A/fromDeviceArray2", arraySize);

  // we need this later anyway, do we make a temporary variable
  auto pvNames = ChimeraTK::getAllVariableNames(csManager);

  DoocsUpdater updater;

  DoocsPVFactory factory(&myEqFct, updater, csManager);

  auto propertyDescriptions = {std::make_shared<SpectrumDescription>("A/fromDeviceArray1", "A", "fromDeviceArray1"),
      std::make_shared<SpectrumDescription>("A/fromDeviceArray2", "A", "fromDeviceArray2")};

  // have the variable created and check that it is the right type
  for(auto const& description : propertyDescriptions) {
    boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create(description);

    // get the raw pointer and dynamic cast it to the expected type
    DoocsSpectrum* doocsSpectrum = dynamic_cast<DoocsSpectrum*>(doocsVariableAsDFct.get());

    // if the cast succeeds the factory works as expected we are done
    BOOST_REQUIRE(doocsSpectrum);
    BOOST_CHECK(static_cast<size_t>(doocsSpectrum->max_length()) == arraySize);
  }
  // FIXME: add tests for x-axis config
}

template<class DOOCS_T>
void testArrayIsCorrectType(DoocsPVFactory& factory,
    ArrayDescription::DataType dataType,
    std::string name = "fromDeviceArray1") {
  auto description = std::make_shared<ArrayDescription>("A/" + name, "A", name, dataType);
  boost::shared_ptr<D_fct> doocsVariableAsDFct = factory.create(description);

  // get the raw pointer and dynamic cast it to the expected type
  DOOCS_T* doocsArray = dynamic_cast<DOOCS_T*>(doocsVariableAsDFct.get());

  // if the cast succeeds the factory works as expected we are done
  BOOST_REQUIRE(doocsArray);
  BOOST_CHECK(static_cast<size_t>(doocsArray->max_length()) == 10);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testCreateArray, T, simple_test_types) {
  std::pair<shared_ptr<ControlSystemPVManager>, shared_ptr<DevicePVManager>> pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 10;
  // PVs can only get one decorator, so we have to put one array for each
  // decorator/doocs type we want to test
  devManager->createProcessArray<T>(deviceToControlSystem, "A/fromDeviceArray1", arraySize);
  devManager->createProcessArray<T>(deviceToControlSystem, "A/fromDeviceArray2", arraySize);
  devManager->createProcessArray<T>(deviceToControlSystem, "A/fromDeviceArray3", arraySize);
  devManager->createProcessArray<T>(deviceToControlSystem, "A/fromDeviceArray4", arraySize);
  devManager->createProcessArray<T>(deviceToControlSystem, "A/fromDeviceArray5", arraySize);
  devManager->createProcessArray<T>(deviceToControlSystem, "A/fromDeviceArray6", arraySize);

  // we need this later anyway, do we make a temporary variable
  auto pvNames = ChimeraTK::getAllVariableNames(csManager);

  DoocsUpdater updater;

  DoocsPVFactory factory(&myEqFct, updater, csManager);

  testArrayIsCorrectType<D_bytearray>(factory, ArrayDescription::DataType::Byte, "fromDeviceArray1");
  testArrayIsCorrectType<D_shortarray>(factory, ArrayDescription::DataType::Short, "fromDeviceArray2");
  testArrayIsCorrectType<D_intarray>(factory, ArrayDescription::DataType::Int, "fromDeviceArray3");
  testArrayIsCorrectType<D_longarray>(factory, ArrayDescription::DataType::Long, "fromDeviceArray4");
  testArrayIsCorrectType<D_floatarray>(factory, ArrayDescription::DataType::Float, "fromDeviceArray5");
  testArrayIsCorrectType<D_doublearray>(factory, ArrayDescription::DataType::Double, "fromDeviceArray6");
}

BOOST_AUTO_TEST_CASE(testAutoCreateArray) {
  std::pair<shared_ptr<ControlSystemPVManager>, shared_ptr<DevicePVManager>> pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 10;
  devManager->createProcessArray<int8_t>(controlSystemToDevice, "A/toDeviceCharArray", arraySize);
  devManager->createProcessArray<uint8_t>(controlSystemToDevice, "A/toDeviceUCharArray", arraySize);
  devManager->createProcessArray<int16_t>(controlSystemToDevice, "A/toDeviceShortArray", arraySize);
  devManager->createProcessArray<uint16_t>(controlSystemToDevice, "A/toDeviceUShortArray", arraySize);
  devManager->createProcessArray<int32_t>(controlSystemToDevice, "A/toDeviceIntArray", arraySize);
  devManager->createProcessArray<uint32_t>(controlSystemToDevice, "A/toDeviceUIntArray", arraySize);
  devManager->createProcessArray<int64_t>(controlSystemToDevice, "A/toDeviceLongArray", arraySize);
  devManager->createProcessArray<uint64_t>(controlSystemToDevice, "A/toDeviceULongArray", arraySize);
  devManager->createProcessArray<float>(controlSystemToDevice, "A/toDeviceFloatArray", arraySize);
  devManager->createProcessArray<double>(controlSystemToDevice, "A/toDeviceDoubleArray", arraySize);

  // we need this later anyway, do we make a temporary variable
  auto pvNames = ChimeraTK::getAllVariableNames(csManager);

  DoocsUpdater updater;

  DoocsPVFactory factory(&myEqFct, updater, csManager);

  testArrayIsCorrectType<D_bytearray>(factory, ArrayDescription::DataType::Auto, "toDeviceCharArray");
  testArrayIsCorrectType<D_bytearray>(factory, ArrayDescription::DataType::Auto, "toDeviceUCharArray");
  testArrayIsCorrectType<D_shortarray>(factory, ArrayDescription::DataType::Auto, "toDeviceShortArray");
  testArrayIsCorrectType<D_shortarray>(factory, ArrayDescription::DataType::Auto, "toDeviceUShortArray");
  testArrayIsCorrectType<D_intarray>(factory, ArrayDescription::DataType::Auto, "toDeviceIntArray");
  testArrayIsCorrectType<D_intarray>(factory, ArrayDescription::DataType::Auto, "toDeviceUIntArray");
  testArrayIsCorrectType<D_longarray>(factory, ArrayDescription::DataType::Auto, "toDeviceLongArray");
  testArrayIsCorrectType<D_longarray>(factory, ArrayDescription::DataType::Auto, "toDeviceULongArray");
  testArrayIsCorrectType<D_floatarray>(factory, ArrayDescription::DataType::Auto, "toDeviceFloatArray");
  testArrayIsCorrectType<D_doublearray>(factory, ArrayDescription::DataType::Auto, "toDeviceDoubleArray");
}

BOOST_AUTO_TEST_CASE(testInt64Scalar) {
  std::pair<shared_ptr<ControlSystemPVManager>, shared_ptr<DevicePVManager>> pvManagers = createPVManager();
  shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  shared_ptr<DevicePVManager> devManager = pvManagers.second;

  devManager->createProcessArray<int64_t>(controlSystemToDevice, "I/toDeviceInt", 1);

  DoocsUpdater updater;

  DoocsPVFactory factory(&myEqFct, updater, csManager);

  ProcessVariable::SharedPtr processScalar = csManager->getProcessArray<int64_t>("I/toDeviceInt");

  auto description = std::make_shared<AutoPropertyDescription>("I/toDeviceInt", "I", "toDeviceInt");
  auto variable64 = factory.create(description);
  // 64 bit integer scalars become D_longarrays because there is no 64 bit
  // scalar representation in Doocs
  BOOST_CHECK(boost::dynamic_pointer_cast<D_longarray>(variable64));
}

// After you finished all test you have to end the test suite.
BOOST_AUTO_TEST_SUITE_END()
