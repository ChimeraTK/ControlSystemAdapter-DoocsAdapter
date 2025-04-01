// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestZMQ_CsToApp

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "DoocsAdapter.h"
#include "fixMissingDoocs_eq_res.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <doocs/EqCall.h>

#include <eq_fct.h>

struct ZmqData {
  doocs::EqData data;
  dmsg_info_t info;
  explicit ZmqData(doocs::EqData* d, dmsg_info_t* i) : info(*i) { data.copy_from(d); }
};

extern const char* object_name;
DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION_WITH_CODE(dmsg_start();)

BOOST_AUTO_TEST_CASE(test_cs_to_app_doocs_scalar) {
  std::cout << "test_cs_to_app_doocs_scalar" << std::endl;

  // TODO: move subscription boilerplate into a fixture.
  // subscribe to property with zmq.
  doocs::EqData dst;
  EqAdr ea;
  ea.adr("doocs://localhost:" + GlobalFixture::rpcNo + "/F/D/TEST_LOCATION/D_SCALAR");
  dmsg_t tag;

  // Wait until server is available
  EqCall eq;
  doocs::EqData s, d;
  size_t counter = 0;
  while(eq.get(&ea, &s, &d) != fixme::comp_code::ok) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if(++counter > 30) {
      BOOST_FAIL("Timeout in waiting for server");
    }
  }

  using Channel = std::promise<ZmqData>;
  Channel channel;
  auto receiver = channel.get_future();
  auto callback = [](void* c, doocs::EqData* data, dmsg_info_t* info) {
    static_cast<Channel*>(c)->set_value(ZmqData{data, info});
  };
  int err = dmsg_attach(&ea, &dst, &channel, callback, &tag);
  BOOST_CHECK(!err);

  // actual test starts here; check if values on a doocs scalar (using
  // the doocs api) propagates to its zmq subscribers.
  DoocsServerTestHelper::doocsSet("//TEST_LOCATION/D_SCALAR", 12);
  if(receiver.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
    auto result = receiver.get();
    BOOST_CHECK_EQUAL(result.data.get_int(), 12);
  }
  else {
    BOOST_FAIL("expected zmq callback timed out");
  }

  dmsg_detach(&ea, tag);
}
