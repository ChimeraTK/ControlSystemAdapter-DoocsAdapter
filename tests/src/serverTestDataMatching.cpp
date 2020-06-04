#define BOOST_TEST_MODULE serverTestDataMatching
#include <boost/test/included/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
//#include <doocs-server-test-helper/ThreadedDoocsServer.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>

extern const char* object_name;
#include <doocs-server-test-helper/ThreadedDoocsServer.h>


using namespace boost::unit_test_framework;
using namespace ChimeraTK;

DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION

/**********************************************************************************************************************/

/// Note: The data is processed by the ReferenceTestApplication in the order
/// of the types as listed in the HolderMap of the ReferenceTestApplication.
/// INT comes before UINT and FLOAT, so the macro pulse number is first
/// written and then our values.

BOOST_AUTO_TEST_CASE(testProcessScalar) {
  std::cout << "testProcessScalar" << std::endl;

  auto appPVmanager = GlobalFixture::referenceTestApplication.getPVManager();

  // We need data consistency between macro pulse number and the data
  // Everything from now on will get the same version number, until we manually
  // set a new one.
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();

  int macroPulseNumber = 12345;
  DoocsServerTestHelper::doocsSet<int>("//UNMAPPED/INT.TO_DEVICE_SCALAR", macroPulseNumber);

  unsigned int expectedUnsignedInt = 42U;
  float expectedFloat = 42.42f;
  DoocsServerTestHelper::doocsSet<unsigned int>("//UINT/TO_DEVICE_SCALAR", expectedUnsignedInt);
  DoocsServerTestHelper::doocsSet<float>("//UNMAPPED/FLOAT.TO_DEVICE_SCALAR", expectedFloat);

  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<unsigned int>("//UINT/FROM_DEVICE_SCALAR") - expectedUnsignedInt < 1e-6);
  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<float>("//UNMAPPED/FLOAT.FROM_DEVICE_SCALAR") - expectedFloat < 1e-6);

  // Send MPN and values with different version numbers
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  auto lastExpectedUnsignedInt = expectedUnsignedInt;
  expectedUnsignedInt = 2U;
  expectedFloat = 2.42f;
  DoocsServerTestHelper::doocsSet<unsigned int>("//UINT/TO_DEVICE_SCALAR", expectedUnsignedInt);
  DoocsServerTestHelper::doocsSet<float>("//UNMAPPED/FLOAT.TO_DEVICE_SCALAR", expectedFloat);

  GlobalFixture::referenceTestApplication.runMainLoopOnce();

  // Unsigned int value must not be updated because it has data matching exact
  // FIXME Better way to check this?
  usleep(1000000);
  BOOST_CHECK_EQUAL(
      DoocsServerTestHelper::doocsGet<unsigned int>("//UINT/FROM_DEVICE_SCALAR"), lastExpectedUnsignedInt);

  // Float value has to get the update even with version number mismatch
  // because data matching is none
  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<float>("//UNMAPPED/FLOAT.FROM_DEVICE_SCALAR") - expectedFloat < 1e-6);

  // Another iteration, now back to consistent version number
  GlobalFixture::referenceTestApplication.versionNumber = ChimeraTK::VersionNumber();
  ++macroPulseNumber;
  DoocsServerTestHelper::doocsSet<int>("//UNMAPPED/INT.TO_DEVICE_SCALAR", macroPulseNumber);
  expectedFloat = 12.42f;
  DoocsServerTestHelper::doocsSet<float>("//UNMAPPED/FLOAT.TO_DEVICE_SCALAR", expectedFloat);
  GlobalFixture::referenceTestApplication.runMainLoopOnce();
  CHECK_WITH_TIMEOUT(
      DoocsServerTestHelper::doocsGet<float>("//UNMAPPED/FLOAT.FROM_DEVICE_SCALAR") - expectedFloat < 1e-6);
}

/**********************************************************************************************************************/
