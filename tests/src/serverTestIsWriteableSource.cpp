// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestIsWriteableSource

#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"

#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <ChimeraTK/UnifiedBackendTest.h> // For CHECK_TIMEOUT

#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>

using namespace boost::unit_test_framework;
using namespace boost::unit_test;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE

BOOST_AUTO_TEST_SUITE(serverTestIsWriteableSource)

/**********************************************************************************************************************/

// Test that properties can be dynamically switched to read-only and read-write
BOOST_AUTO_TEST_CASE(TestDynamicSwitching) {
  const std::string mainPropName = "//DYNAMIC/PROP";
  const std::string switchPropName = "//DYNAMIC/SWITCH";

  auto* location = getLocationFromPropertyAddress(mainPropName);

  auto* mainProp = getDoocsProperty<D_int>(mainPropName);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 0);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RO), 30000);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 1);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RW), 30000);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 0);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RO), 30000);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 1);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RW), 30000);
}

/**********************************************************************************************************************/

// Test that properties can be dynamically switched to read-only and read-write
BOOST_AUTO_TEST_CASE(TestDynamicSwitchingToDevice) {
  const std::string mainPropName = "//DYNAMIC_TO_DEVICE/PROP";
  const std::string switchPropName = "//DYNAMIC_TO_DEVICE/SWITCH";

  auto* location = getLocationFromPropertyAddress(mainPropName);

  auto* mainProp = getDoocsProperty<D_int>(mainPropName);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 0);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RO), 30000);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 1);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RW), 30000);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 0);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RO), 30000);

  DoocsServerTestHelper::doocsSet<int>(switchPropName, 1);
  DoocsServerTestHelper::runUpdate();
  CHECK_TIMEOUT((std::unique_lock<EqFct>(*location), mainProp->get_access() == ACCESS_RW), 30000);
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
