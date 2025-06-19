// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestZeroMQ_noMatchingMPNumber

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "ZeroMQFixture.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

/**********************************************************************************************************************/

BOOST_AUTO_TEST_SUITE(serverTestZeroMQ_noMatchingMPNumber)

BOOST_FIXTURE_TEST_CASE(testScalar, ZeroMQFixture<int32_t>) {
  std::cout << "testScalar " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  toDevicePath = "//INT/TO_DEVICE_SCALAR";
  fromDevicePath = "//INT/FROM_DEVICE_SCALAR";
  configUsesDataMatching = false;

  init();

  checkTransport();
}

// CS2CS variant tests data updates bypassing the Application module
BOOST_FIXTURE_TEST_CASE(testScalarCS2CS, ZeroMQFixture<int32_t>) {
  std::cout << "testScalarCS2CS " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  toDevicePath = "//INT/TO_DEVICE_SCALAR";
  fromDevicePath = toDevicePath;
  configUsesDataMatching = false;
  // we cannot generate errors since data not routed through AC app
  generateErrors = false;

  init();

  checkTransport();
}

BOOST_FIXTURE_TEST_CASE(testArray, ZeroMQFixture<std::vector<int32_t>>) {
  std::cout << "testArray " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  toDevicePath = "//INT/TO_DEVICE_ARRAY";
  fromDevicePath = "//INT/FROM_DEVICE_ARRAY";
  configUsesDataMatching = false;
  init();

  checkTransport();
}

BOOST_FIXTURE_TEST_CASE(testArrayCS2CS, ZeroMQFixture<std::vector<int32_t>>) {
  std::cout << "testArrayCS2CS " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  toDevicePath = "//INT/TO_DEVICE_ARRAY";
  fromDevicePath = toDevicePath;
  configUsesDataMatching = false;
  // we cannot generate errors since data not routed through AC app
  generateErrors = false;
  init();

  checkTransport();
}

BOOST_FIXTURE_TEST_CASE(testIFFFa, ZeroMQFixture<IFFF>) {
  std::cout << "testIFFFa" << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  toDevicePath = "//CUSTOM/IFFF";
  fromDevicePath = toDevicePath;
  fromDeviceMetaDataPath = "//INT/TO_DEVICE_SCALAR";
  configUsesDataMatching = false;
  // we cannot generate errors since data not routed through AC app
  generateErrors = false;

  init();
  checkTransport();
}

using FixtureForTestIFFFb = ZeroMQFixture<IFFF, int>; // needed because of ',' parsing in macro
BOOST_FIXTURE_TEST_CASE(testIFFFb, FixtureForTestIFFFb) {
  std::cout << "testIFFFb" << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  toDevicePath = "//CUSTOM/IFFF";
  fromDevicePath = "//INT/TO_DEVICE_SCALAR";
  configUsesDataMatching = false;
  // we cannot generate errors since data not routed through AC app
  generateErrors = false;

  init();
  checkTransport();
}

BOOST_AUTO_TEST_SUITE_END()
