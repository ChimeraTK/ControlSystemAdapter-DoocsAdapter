// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestSpectrumBuffer - exception
#include <doocs-server-test-helper/doocsServerTestHelper.h>

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <boost/test/included/unit_test.hpp>
extern const char* object_name;

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <eq_client.h>

#include <random>
#include <thread>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

static ReferenceTestApplication referenceTestApplication("serverTestSpectrumBuffer-exception");

/**********************************************************************************************************************/

extern int eq_server(int, char**);

/**********************************************************************************************************************/

BOOST_AUTO_TEST_CASE(testSpectrum) {
  // choose random RPC number
  std::random_device rd;
  std::uniform_int_distribution<int> dist(620000000, 999999999);
  auto rpc_no = std::to_string(dist(rd));
  // update config file with the RPC number
  std::string command = "sed -i serverTestSpectrumBuffer-exception.conf -e "
                        "'s/^SVR.RPC_NUMBER:.*$/SVR.RPC_NUMBER: " +
      rpc_no + "/'";
  auto rc = std::system(command.c_str());
  (void)rc;

  // staring the server should cause the exception
  try {
    object_name = "serverTestSpectrumBuffer-exception";
    eq_server(
        boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
    BOOST_ERROR("Exception expected");
  }
  catch(ChimeraTK::logic_error&) {
  }
}

/**********************************************************************************************************************/
