// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestZeroMQ

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "ZeroMQFixture.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

BOOST_AUTO_TEST_SUITE(ZeroMQTestSuite)

/**********************************************************************************************************************/

BOOST_FIXTURE_TEST_CASE(testZMQScalar, ZeroMQFixture<int32_t>) {
  std::cout << "testZMQScalar " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  macroPulseNumber = 12345;
  toDevicePath = "//INT/TO_DEVICE_SCALAR";
  fromDevicePath = "//INT/FROM_DEVICE_SCALAR";
  init();

  checkTransport();
}

/**********************************************************************************************************************/

BOOST_FIXTURE_TEST_CASE(testZMQArray, ZeroMQFixture<std::vector<int32_t>>) {
  std::cout << "testZMQArray " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  macroPulseNumber = 99999;
  toDevicePath = "//INT/TO_DEVICE_ARRAY";
  fromDevicePath = "//INT/FROM_DEVICE_ARRAY";

  init();
  checkTransport();
}

/**********************************************************************************************************************/

BOOST_FIXTURE_TEST_CASE(testZMQSpectrum, ZeroMQFixture<std::vector<float>>) {
  std::cout << "testZMQSpectrum " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  macroPulseNumber = 100;
  toDevicePath = "//FLOAT/TO_DEVICE_ARRAY";
  fromDevicePath = "//FLOAT/FROM_DEVICE_ARRAY";

  init();
  checkTransport();
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
