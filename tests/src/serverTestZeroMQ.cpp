// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "doocs/EqCall.h"
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

// TODO add tests for new DOOCS protocol, DOOCS-over-ZeroMQ
// plan:
// look at doocs4py test code of following ticket,
//  https://rt-system.desy.de/Ticket/Display.html?id=1630892
// but rewrite in C++ - that should be possible since doocs4py is just a binding of C++ API to python.
// We don't want another dependency, doocs4py, in our project
//
// compare commit of pbm server 'assign proper timestamp and event_id to D_BPMspectrum data'
// https://mcs-gitlab.desy.de/controls-and-operation/particle-beam-diagnostics/beam-position-measurement/bpm/-/commit/88a661428d9556a3a27f17335d905c81354a6df8?merge_request_iid=5
//
// We might have  a problem with port mapping here.
// Usually, port numbers are found via ENS registry, but we don't like to depend on that for unit testing.
// For classic RPC-based DOOCS, we have solved that problem by explicitly mentioning RPC port number in address,
// like doocs://localhost:<rpcnum>/F/D/L/P .
// But for DOOCS-over-ZeroMQ, we do not know in advance the involved port numbers (one for sync, one for async
// protocol). How would we find them? => parse stdout of server
// Additionally, how would we specify the port numbers in the client?

void f(unsigned port) {
  doocs::EqCall eq;

  // doocs::EqAdr addr("doocszmq://xfelcpusd181b1:0/XFEL.SDIAG/BCM/BCM.180.B1.1/COMPRESSION.TD");

  doocs::EqAdr addr(
      "doocszmq://xfelcpusd181b1:" + std::to_string(port) + "/XFEL.SDIAG/BCM/BCM.180.B1.1/COMPRESSION.TD");
  std::cout << "testing subscription to  " << addr.show_adr() << std::endl;

  EqData src, data_out;
  eq.get_option(&addr, &src, &data_out, EqOption::EQ_PROTOCOL);
  std::cout << "protocol=" << data_out.get_string() << std::endl;

  auto callback = [](doocs::EqData& data) { std::cout << "received msg event " << data.get_event_id() << std::endl; };
  auto subscription = eq.subscribe(addr, callback);
  sleep(10);
}
BOOST_AUTO_TEST_CASE(testDoocsZmq) {
  f(35339); // sync port
  std::cout << "1 end of testDoocsZmq" << std::endl;
  f(36151); // async port
  std::cout << "2 end of testDoocsZmq" << std::endl;
  f(36152); // invalid
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
