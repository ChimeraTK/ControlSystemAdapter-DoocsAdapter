// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE DoocsSpectrumTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include "DoocsSpectrum.h"

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <ChimeraTK/TypeChangingDecorator.h>

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <limits>
#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

class TestableDoocsSpectrum : public DoocsSpectrum {
 public:
  TestableDoocsSpectrum(EqFct* const eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
      DataConsistencyGroup::MatchingMode matchingMode)
  : DoocsSpectrum(eqFct, doocsPropertyName, processArray, updater, matchingMode, nullptr, nullptr) {}

  using DoocsSpectrum::sendToDevice;
};

// use boost meta-programming to use test case templates
// The list of types is an mpl type
using simple_test_types = boost::mpl::list<int32_t, uint32_t, int16_t, uint16_t, int8_t, uint8_t, float, double>;

BOOST_AUTO_TEST_SUITE(DoocsSpectrumTestSuite)

BOOST_AUTO_TEST_CASE_TEMPLATE(toDeviceTest, T, simple_test_types) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 8;
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> deviceVariable =
      devManager->createProcessArray<T>(SynchronizationDirection::controlSystemToDevice, "toDeviceVariable", arraySize);
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> controlSystemVariable =
      csManager->getProcessArray<T>("toDeviceVariable");

  DoocsUpdater updater;

  // Write to the doocs spectrum and send it.
  // We use the 'testable' version which exposes sendToDevice, which otherwise
  // is protected.
  TestableDoocsSpectrum doocsSpectrum(nullptr, "someName", getTypeChangingDecorator<float>(controlSystemVariable),
      updater, DataConsistencyGroup::MatchingMode::exact);

  // create unique signature for each template parameter
  // negative factor for signed values
  T sign = (std::numeric_limits<T>::is_signed ? -1 : 1);
  // integer size offset for integer, fractional offset for floating type
  T offset = (std::numeric_limits<T>::is_integer ? sizeof(T) : 1. / sizeof(T));

  for(size_t i = 0; i < arraySize; ++i) {
    doocsSpectrum.fill_spectrum(i, sign * static_cast<T>(i * i) + offset);
  }
  doocsSpectrum.sendToDevice(false);

  // receive on the device side and check that the value has arrived
  deviceVariable->readNonBlocking();

  std::vector<T>& deviceVector = deviceVariable->accessChannel(0);
  for(size_t i = 0; i < arraySize; ++i) {
    std::stringstream errorMessage;
    errorMessage << "i = " << i << ", deviceVector[i] = " << deviceVector[i] << " expected "
                 << sign * static_cast<T>(i * i) + offset;
    BOOST_CHECK_MESSAGE(deviceVector[i] == sign * static_cast<T>(i * i) + offset, errorMessage.str());
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(fromDeviceTest, T, simple_test_types) {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  static const size_t arraySize = 8;
  typename boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> deviceVariable = devManager->createProcessArray<T>(
      SynchronizationDirection::deviceToControlSystem, "fromDeviceVariable", arraySize);
  typename boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> controlSystemVariable =
      csManager->getProcessArray<T>("fromDeviceVariable");

  DoocsUpdater updater;

  // initialise the doocs spectrum
  DoocsSpectrum doocsSpectrum(nullptr, "someName", getTypeChangingDecorator<float>(controlSystemVariable), updater,
      DataConsistencyGroup::MatchingMode::exact, nullptr, nullptr);
  for(size_t i = 0; i < arraySize; ++i) {
    doocsSpectrum.fill_spectrum(i, 0);
  }

  // prepare the device side and send it
  T sign = (std::numeric_limits<T>::is_signed ? -1 : 1);
  T offset = (std::numeric_limits<T>::is_integer ? sizeof(T) : 1. / sizeof(T));

  std::vector<T>& deviceVector = deviceVariable->accessChannel(0);
  for(size_t i = 0; i < arraySize; ++i) {
    deviceVector[i] = (sign * static_cast<T>(i * i) + offset);
  }
  deviceVariable->write();

  // everything should still be 0 on the CS side
  std::vector<T>& csVector = controlSystemVariable->accessChannel(0);
  for(size_t i = 0; i < arraySize; ++i) {
    BOOST_CHECK(csVector[i] == 0);
    BOOST_CHECK(doocsSpectrum.read_spectrum(i) == 0);
  }

  updater.update();

  // The actual vector buffer has changed. We have to get the new reference.
  csVector = controlSystemVariable->accessChannel(0);
  for(size_t i = 0; i < arraySize; ++i) {
    BOOST_CHECK(csVector[i] == sign * static_cast<T>(i * i) + offset);
    BOOST_CHECK_EQUAL(doocsSpectrum.read_spectrum(i), sign * static_cast<T>(i * i) + offset);
  }
}

BOOST_AUTO_TEST_SUITE_END()
