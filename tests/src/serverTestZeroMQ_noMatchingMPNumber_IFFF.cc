// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestZeroMQ_noMatchingMPNumber_IFFF

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "ZeroMQFixture.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

/**********************************************************************************************************************/

BOOST_AUTO_TEST_SUITE(serverTestZeroMQ_noMatchingMPNumber_IFFF)

BOOST_FIXTURE_TEST_CASE(testIFFFa, ZeroMQFixture<IFFF>) {
  std::cout << "testIFFFa " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

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
  std::cout << "testIFFFb " << GlobalFixture::rpcNo << " " << GlobalFixture::bpn << std::endl;

  // FIXME: This test is re-using writable process variables which are already mapped for other tests. We should only
  // map PVs once as writable.

  toDevicePath = "//CUSTOM/IFFF";
  fromDevicePath = "//INT/FROM_DEVICE_SCALAR";
  configUsesDataMatching = false;
  // we cannot generate errors since data not routed through AC app
  generateErrors = false;

  init();
  checkTransport();
}

BOOST_AUTO_TEST_SUITE_END()
