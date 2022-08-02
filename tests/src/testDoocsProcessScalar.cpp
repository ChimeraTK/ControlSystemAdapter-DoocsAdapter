// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE DoocsProcessScalarTest
// Only after defining the name include the unit test header.
#include <boost/mpl/list.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>

#include "DoocsProcessScalar.h"
#include "DoocsUpdater.h"
#include "D_textUnifier.h"
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <d_fct.h>

#include "emptyServerFunctions.h"
#include "set_doocs_value.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

// use boost meta-programming to use test case templates
// The list of types is an mpl type
typedef boost::mpl::list<int32_t, uint32_t, int16_t, uint16_t, int8_t, uint8_t> integer_test_types;

EqFct myLocation;

BOOST_AUTO_TEST_SUITE(DoocsProcessScalarTestSuite)

BOOST_AUTO_TEST_CASE_TEMPLATE(toDeviceIntegerTypeTest, T, integer_test_types) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> deviceVariable =
      devManager->createProcessArray<T>(SynchronizationDirection::controlSystemToDevice, "toDeviceVariable", 1);
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> controlSystemVariable =
      csManager->getProcessArray<T>("toDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0) = 0;
  controlSystemVariable->accessData(0) = 0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<T, D_int> doocsScalar(&myLocation, "TO_DEVICE_VARIABLE", controlSystemVariable, updater);

  BOOST_CHECK(set_doocs_value(doocsScalar, 42) == 0);
  BOOST_CHECK(controlSystemVariable->accessData(0) == 42);

  // receive on the device side and check that the value has arrived
  deviceVariable->readNonBlocking();
  BOOST_CHECK(deviceVariable->accessData(0) == 42);

  // check with negative values, and cast so unsigned gets correct results

  // check that the set() overloading is working by calling the function of the
  // base class (note: cast to a reference, otherwise inheritance/ virtual
  // functions calls do not work)
  BOOST_CHECK(set_doocs_value(static_cast<D_int&>(doocsScalar), -13.) == 0);
  BOOST_CHECK(controlSystemVariable->accessData(0) == static_cast<T>(-13));

  // receive on the device side and check that the value has arrived
  deviceVariable->readNonBlocking();
  BOOST_CHECK(deviceVariable->accessData(0) == static_cast<T>(-13));
}

// as boost testing does not allow multiple template parameters we use code
// duplication to do DoocsProcessScalar<float, D_float, float> and
// DoocsProcessScalar<double, D_double, double>
BOOST_AUTO_TEST_CASE(toDeviceFloatTest) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  boost::shared_ptr<ProcessArray<float>> deviceFloat =
      devManager->createProcessArray<float>(SynchronizationDirection::controlSystemToDevice, "toDeviceFloat", 1);
  boost::shared_ptr<ProcessArray<float>> controlSystemFloat = csManager->getProcessArray<float>("toDeviceFloat");
  // set the variables to 0
  deviceFloat->accessData(0) = 0;
  controlSystemFloat->accessData(0) = 0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<float, D_float> doocsScalar(&myLocation, "TO_DEVICE_FLOAT", controlSystemFloat, updater);

  BOOST_CHECK(set_doocs_value(doocsScalar, 12.125) == 0);
  BOOST_CHECK_CLOSE(controlSystemFloat->accessData(0), 12.125, 0.00001);

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK_CLOSE(deviceFloat->accessData(0), 12.125, 0.00001);

  // check that the value() overloading is working by calling the function of
  // the base class (note: cast to a reference, otherwise inheritance/ virtual
  // functions calls do not work)
  BOOST_CHECK(set_doocs_value(static_cast<D_float&>(doocsScalar), -13.) == 0);
  BOOST_CHECK_CLOSE(controlSystemFloat->accessData(0), -13., 0.00001);

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK_CLOSE(deviceFloat->accessData(0), -13., 0.00001);
}

BOOST_AUTO_TEST_CASE(toDeviceDoubleTest) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  boost::shared_ptr<ProcessArray<double>> deviceDouble =
      devManager->createProcessArray<double>(SynchronizationDirection::controlSystemToDevice, "toDeviceDouble", 1);
  boost::shared_ptr<ProcessArray<double>> controlSystemDouble = csManager->getProcessArray<double>("toDeviceDouble");
  // set the variables to 0
  deviceDouble->accessData(0) = 0;
  controlSystemDouble->accessData(0) = 0;

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<double, D_double> doocsScalar(&myLocation, "TO_DEVICE_DOUBLE", controlSystemDouble, updater);

  BOOST_CHECK(set_doocs_value(doocsScalar, 12.125) == 0);
  BOOST_CHECK(controlSystemDouble->accessData(0) == 12.125);

  // receive on the device side and check that the value has arrived
  deviceDouble->readNonBlocking();
  BOOST_CHECK(deviceDouble->accessData(0) == 12.125);

  // check that the value() overloading is working by calling the function of
  // the base class (note: cast to a reference, otherwise inheritance/ virtual
  // functions calls do not work)
  BOOST_CHECK(set_doocs_value(static_cast<D_double&>(doocsScalar), -13.) == 0);
  BOOST_CHECK_CLOSE(controlSystemDouble->accessData(0), -13., 0.00001);

  // receive on the device side and check that the value has arrived
  deviceDouble->readNonBlocking();
  BOOST_CHECK_CLOSE(deviceDouble->accessData(0), -13., 0.00001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(fromDeviceIntegerTypeTest, T, integer_test_types) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  typename boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> deviceVariable =
      devManager->createProcessArray<T>(SynchronizationDirection::deviceToControlSystem, "fromDeviceVariable", 1);
  typename boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> controlSystemVariable =
      csManager->getProcessArray<T>("fromDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0) = 0;
  controlSystemVariable->accessData(0) = 0;

  // initialise the doocs scalar
  DoocsProcessScalar<T, D_int> doocsScalar(&myLocation, "FROM_DEVICE_VARIABLE", controlSystemVariable, updater);

  deviceVariable->accessData(0) = 42;
  deviceVariable->write();

  BOOST_CHECK(controlSystemVariable->accessData(0) == 0);
  BOOST_CHECK(doocsScalar.value() == 0);

  updater.update();
  BOOST_CHECK(controlSystemVariable->accessData(0) == 42);
  BOOST_CHECK(doocsScalar.value() == 42);

  // negative test for signed int, with cast for uints
  deviceVariable->accessData(0) = -13;
  deviceVariable->write();
  updater.update();
  BOOST_CHECK(doocsScalar.value() == static_cast<int>(static_cast<T>(-13)));
}

BOOST_AUTO_TEST_CASE(toDeviceStringTest) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  boost::shared_ptr<ProcessArray<std::string>> deviceFloat =
      devManager->createProcessArray<std::string>(SynchronizationDirection::controlSystemToDevice, "toDeviceString", 1);
  boost::shared_ptr<ProcessArray<std::string>> controlSystemFloat =
      csManager->getProcessArray<std::string>("toDeviceString");
  // set the variables to 0
  deviceFloat->accessData(0) = "null";
  controlSystemFloat->accessData(0) = "null";

  // just write to the doocs scalar, it is automatically sending
  DoocsProcessScalar<std::string, D_textUnifier> doocsScalar(
      &myLocation, "TO_DEVICE_STRING", controlSystemFloat, updater);

  BOOST_CHECK(set_doocs_value(doocsScalar, "twelvepointonetwofive") == 0);
  BOOST_CHECK(controlSystemFloat->accessData(0) == "twelvepointonetwofive");

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK(deviceFloat->accessData(0) == "twelvepointonetwofive");

  // check that the value() overloading is working by calling the function of
  // the base class (note: cast to a reference, otherwise inheritance/ virtual
  // functions calls do not work)
  BOOST_CHECK(set_doocs_value(static_cast<D_textUnifier&>(doocsScalar), "minusthirteen") == 0);
  BOOST_CHECK(controlSystemFloat->accessData(0) == "minusthirteen");

  // receive on the device side and check that the value has arrived
  deviceFloat->readNonBlocking();
  BOOST_CHECK(deviceFloat->accessData(0) == "minusthirteen");
}

BOOST_AUTO_TEST_CASE(fromDeviceStringTest) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  ProcessArray<std::string>::SharedPtr deviceVariable = devManager->createProcessArray<std::string>(
      SynchronizationDirection::deviceToControlSystem, "fromDeviceVariable", 1);
  ProcessArray<std::string>::SharedPtr controlSystemVariable =
      csManager->getProcessArray<std::string>("fromDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0) = "null";
  controlSystemVariable->accessData(0) = "null";

  // initialise the doocs scalar
  DoocsProcessScalar<std::string, D_textUnifier> doocsScalar(
      nullptr, "FROM_DEVICE_VARIABLE", controlSystemVariable, updater);

  deviceVariable->accessData(0) = "twelvepointonetwofive";
  deviceVariable->write();

  BOOST_CHECK_EQUAL(controlSystemVariable->accessData(0), "null");
  BOOST_CHECK_EQUAL(doocsScalar.value(), "");

  updater.update();
  BOOST_CHECK_EQUAL(controlSystemVariable->accessData(0), "twelvepointonetwofive");
  BOOST_CHECK_EQUAL(doocsScalar.value(), "twelvepointonetwofive");
}

BOOST_AUTO_TEST_CASE(fromDeviceFloatTest) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  ProcessArray<float>::SharedPtr deviceVariable =
      devManager->createProcessArray<float>(SynchronizationDirection::deviceToControlSystem, "fromDeviceVariable", 1);
  ProcessArray<float>::SharedPtr controlSystemVariable = csManager->getProcessArray<float>("fromDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0) = 0.0;
  controlSystemVariable->accessData(0) = 0.0;

  // initialise the doocs scalar
  DoocsProcessScalar<float, D_float> doocsScalar(&myLocation, "FROM_DEVICE_VARIABLE", controlSystemVariable, updater);

  deviceVariable->accessData(0) = 12.125;
  deviceVariable->write();

  BOOST_CHECK_CLOSE(controlSystemVariable->accessData(0), 0.0, 0.00001);
  BOOST_CHECK_CLOSE(doocsScalar.value(), 0.0, 1e-6);

  updater.update();
  BOOST_CHECK_CLOSE(controlSystemVariable->accessData(0), 12.125, 0.00001);
  BOOST_CHECK_CLOSE(doocsScalar.value(), 12.125, 0.00001);
}

BOOST_AUTO_TEST_CASE(fromDeviceDoubleTest) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  DoocsUpdater updater;

  ProcessArray<double>::SharedPtr deviceVariable =
      devManager->createProcessArray<double>(SynchronizationDirection::deviceToControlSystem, "fromDeviceVariable", 1);
  ProcessArray<double>::SharedPtr controlSystemVariable = csManager->getProcessArray<double>("fromDeviceVariable");
  // set the variables to 0
  deviceVariable->accessData(0) = 0;
  controlSystemVariable->accessData(0) = 0;

  // initialise the doocs scalar
  DoocsProcessScalar<double, D_double> doocsScalar(&myLocation, "FROM_DEVICE_VARIABLE", controlSystemVariable, updater);

  deviceVariable->accessData(0) = 12.125;
  deviceVariable->write();

  BOOST_CHECK_CLOSE(controlSystemVariable->accessData(0), 0, 0.00001);
  BOOST_CHECK_CLOSE(doocsScalar.value(), 0, 0.00001);

  updater.update();
  BOOST_CHECK_CLOSE(controlSystemVariable->accessData(0), 12.125, 0.00001);
  BOOST_CHECK_CLOSE(doocsScalar.value(), 12.125, 0.00001);
}

BOOST_AUTO_TEST_SUITE_END()
