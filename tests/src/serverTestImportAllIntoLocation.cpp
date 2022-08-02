// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestImportAllIntoLocation
#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

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

/// Check that all expected variables are there.
BOOST_AUTO_TEST_CASE(testVariableExistence) {
  //  for (auto const directory : { "CHAR", "DOUBLE", "FLOAT", "INT", "SHORT",
  //  "UCHAR", "UINT", "USHORT"} ){
  for(auto const directory : {"DOUBLE", "FLOAT", "INT", "SHORT", "UCHAR", "UINT", "USHORT", "CHAR"}) {
    for(auto const variable : {"CONSTANT_ARRAY", "FROM_DEVICE_ARRAY", "TO_DEVICE_ARRAY "}) {
      // if this throws the property does not exist. we should always be able to
      // read"
      std::cout << "testing existence of " << std::string("//MASTER/") + directory + "." + variable << std::endl;
      BOOST_CHECK_NO_THROW(
          DoocsServerTestHelper::doocsGetArray<int>((std::string("//MASTER/") + directory + "." + variable).c_str()));
    }
    for(auto const variable : {"DATA_TYPE_CONSTANT", "FROM_DEVICE_SCALAR", "TO_DEVICE_SCALAR"}) {
      std::cout << "testing existence of " << std::string("//MASTER/") + directory + "." + variable << std::endl;
      // if this throws the property does not exist. we should always be able to
      // read"
      BOOST_CHECK_NO_THROW(
          DoocsServerTestHelper::doocsGet<int>((std::string("//MASTER/") + directory + "." + variable).c_str()));
      std::cout << "test done " << std::endl;
    }
  }
}
