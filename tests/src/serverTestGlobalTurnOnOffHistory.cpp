// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestGlobalTurnOnOffHistory
#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>

#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE

BOOST_AUTO_TEST_SUITE(serverTestGlobalTurnOnOffHistory)

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(testVariableExistence) {
  usleep(100000);

  // the stuff with default. We are lazy and put the integer types as we have to
  // list D_double and D_float separately anyway if we don't want to do
  // meta-programming
  for(const auto& location : {"SHORT", "USHORT", "CHAR", "UCHAR", "INT", "UINT"}) {
    std::cout << "testing " << location << std::endl;
    checkDoocsProperty<D_int>(std::string("//") + location + "/DATA_TYPE_CONSTANT", false, false);
    checkDoocsProperty<D_int>(std::string("//") + location + "/FROM_DEVICE_SCALAR", false, false);
    checkDoocsProperty<D_int>(std::string("//") + location + "/TO_DEVICE_SCALAR", false, true);
  }
  for(const auto& nameWriteable :
      {std::make_pair("CONSTANT_ARRAY", false), {"FROM_DEVICE_ARRAY", false}, {"TO_DEVICE_ARRAY", true}}) {
    checkDoocsProperty<D_intarray>(std::string("//INT/") + nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_intarray>(std::string("//UINT/") + nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_bytearray>(std::string("//CHAR/") + nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_bytearray>(std::string("//UCHAR/") + nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_shortarray>(std::string("//SHORT/") + nameWriteable.first, false, nameWriteable.second);
    checkDoocsProperty<D_shortarray>(std::string("//USHORT/") + nameWriteable.first, false, nameWriteable.second);
  }

  std::cout << "testing DOUBLE" << std::endl;
  checkDoocsProperty<D_doublearray>("//DOUBLE/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/TO_DEVICE_ARRAY", true);
  checkDoocsProperty<D_double>("//DOUBLE/DATA_TYPE_CONSTANT", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/TO_DEVICE_SCALAR", false, true);

  std::cout << "testing FLOAT" << std::endl;
  checkDoocsProperty<D_floatarray>("//FLOAT/CONSTANT_ARRAY", false, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/FROM_DEVICE_ARRAY", false, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/TO_DEVICE_ARRAY");
  checkDoocsProperty<D_float>("//FLOAT/DATA_TYPE_CONSTANT", false, false);
  checkDoocsProperty<D_float>("//FLOAT/FROM_DEVICE_SCALAR", false, false);
  checkDoocsProperty<D_float>("//FLOAT/TO_DEVICE_SCALAR", true, true);
}

BOOST_AUTO_TEST_SUITE_END()
