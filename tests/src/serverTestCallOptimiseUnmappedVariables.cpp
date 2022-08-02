// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestCallOptimiseUnmappedVariables

#include <boost/filesystem.hpp>

#include "DoocsAdapter.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <thread>
#include <algorithm>

#define BOOST_NO_EXCEPTIONS
#include <boost/test/included/unit_test.hpp>
#undef BOOST_NO_EXCEPTIONS

#include "serverBasedTestTools.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE

BOOST_AUTO_TEST_CASE(testListOfUnmappedVariables) {
  std::list<std::string> refAppModuleList = {"CHAR", "UCHAR", "SHORT", "USHORT", "INT", "UINT", "LONG", "ULONG",
                                             "FLOAT", "DOUBLE", "STRING"};
  std::list<std::string> refAppVarPerModuleList = {"TO_DEVICE_SCALAR", "FROM_DEVICE_SCALAR", "TO_DEVICE_ARRAY",
                                                   "FROM_DEVICE_ARRAY", "DATA_TYPE_CONSTANT", "CONSTANT_ARRAY"};
  std::set<std::string> mappedVars = {"/DOUBLE/TO_DEVICE_ARRAY", "/FLOAT/TO_DEVICE_SCALAR", "/FLOAT/TO_DEVICE_SCALAR",
                                      "/DOUBLE/TO_DEVICE_SCALAR", "/DOUBLE/FROM_DEVICE_ARRAY",
                                      "/FLOAT/TO_DEVICE_SCALAR"};
  std::set<std::string> expectedList;
  std::cout << "=== Expected list: " << std::endl;
  for(auto &m : refAppModuleList) {
    for(auto &v : refAppVarPerModuleList) {
      std::string name = "/"+m+"/"+v;
      if(mappedVars.find(name) == mappedVars.end()) {
        std::cout << name << std::endl;
        expectedList.insert(name);
      }
    }
  }

  std::cout << "=== Actual list: " << std::endl;
  auto &actualList = GlobalFixture::referenceTestApplication._unmappedVariables;
  for(auto &name : actualList) {
    std::cout << name << std::endl;
  }

  std::cout << "=== Missing entries: " << std::endl;
  std::set<std::string> missing;
  std::set_difference(expectedList.begin(), expectedList.end(), actualList.begin(), actualList.end(),
                      std::inserter(missing, missing.begin()));
  for(auto &name : missing) {
    std::cout << name << std::endl;
  }

  std::cout << "=== Additional entries: " << std::endl;
  std::set<std::string> additional;
  std::set_difference(actualList.begin(), actualList.end(), expectedList.begin(), expectedList.end(),
                      std::inserter(additional, additional.begin()));
  for(auto &name : additional) {
    std::cout << name << std::endl;
  }

  BOOST_CHECK(actualList == expectedList);
}
