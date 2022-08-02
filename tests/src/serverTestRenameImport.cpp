// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_MODULE serverTestRenameImport
#include <boost/test/included/unit_test.hpp>

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
  checkDoocsProperty<D_intarray>("//MY_RENAMED_INTEGER_LOCATION/RENAMED.CONST_ARRAY", true, false);
  checkDoocsProperty<D_intarray>("//MY_RENAMED_INTEGER_LOCATION/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_intarray>("//MY_RENAMED_INTEGER_LOCATION/TO_DEVICE_ARRAY", true, true);
  checkDoocsProperty<D_int>("//MY_RENAMED_INTEGER_LOCATION/DATA_TYPE_CONSTANT", true, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//MY_RENAMED_INTEGER_LOCATION/DATA_TYPE_CONSTANT") == -4);
  checkDoocsProperty<D_int>("//MY_RENAMED_INTEGER_LOCATION/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//MY_RENAMED_INTEGER_LOCATION/TO_DEVICE_SCALAR", true, true);

  checkDoocsProperty<D_shortarray>("//SHORT/myStuff.CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_shortarray>("//SHORT/myStuff.FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_shortarray>("//SHORT/myStuff.TO_DEVICE_ARRAY");
  checkDoocsProperty<D_int>("//SHORT/myStuff.DATA_TYPE_CONSTANT", true, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//SHORT/myStuff.DATA_TYPE_CONSTANT") == -2);
  checkDoocsProperty<D_int>("//SHORT/myStuff.FROM_DEVICE_SCALAR", true, false);

  checkDoocsProperty<D_int>("//CHERRY_PICKED/TO_DEVICE_SHORT");

  checkDoocsProperty<D_doublearray>("//DOUBLE/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_doublearray>("//DOUBLE/TO_DEVICE_ARRAY");
  checkDoocsProperty<D_double>("//DOUBLE/RENAMED_CONSTANT", false, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<double>("//DOUBLE/RENAMED_CONSTANT") == 1. / 8.);
  checkDoocsProperty<D_double>("//DOUBLE/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_double>("//DOUBLE/DOUBLE.TO_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_float>("//DOUBLE/I_AM_A_FLOAT_SCALAR");

  // we moved one float scalar to the double location, so we cannot check for it
  // in the loop above
  checkDoocsProperty<D_floatarray>("//FLOAT/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_floatarray>("//FLOAT/TO_DEVICE_ARRAY");
  checkDoocsProperty<D_float>("//FLOAT/DATA_TYPE_CONSTANT", true, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<float>("//FLOAT/DATA_TYPE_CONSTANT") == 1. / 4.);
  checkDoocsProperty<D_float>("//FLOAT/FROM_DEVICE_SCALAR", true, false);

  checkDoocsProperty<D_intarray>("//UINT/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_intarray>("//UINT/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_intarray>("//UINT/TO_DEVICE_ARRAY");
  checkDoocsProperty<D_int>("//UINT/DATA_TYPE_CONSTANT", true, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//UINT/DATA_TYPE_CONSTANT") == 4);
  checkDoocsProperty<D_int>("//UINT/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//UINT/TO_DEVICE_SCALAR");

  checkDoocsProperty<D_shortarray>("//USHORT/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_shortarray>("//USHORT/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_shortarray>("//USHORT/TO_DEVICE_ARRAY");
  checkDoocsProperty<D_int>("//USHORT/DATA_TYPE_CONSTANT", false, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//USHORT/DATA_TYPE_CONSTANT") == 2);
  checkDoocsProperty<D_int>("//USHORT/FROM_DEVICE_SCALAR", false, false);
  checkDoocsProperty<D_int>("//USHORT/TO_DEVICE_SCALAR", true, true);

  checkDoocsProperty<D_bytearray>("//UCHAR/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_bytearray>("//UCHAR/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_bytearray>("//UCHAR/TO_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_int>("//UCHAR/DATA_TYPE_CONSTANT", true, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//UCHAR/DATA_TYPE_CONSTANT") == 1);
  checkDoocsProperty<D_int>("//UCHAR/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//UCHAR/TO_DEVICE_SCALAR");

  checkDoocsProperty<D_bytearray>("//CHAR/CONSTANT_ARRAY", true, false);
  checkDoocsProperty<D_bytearray>("//CHAR/FROM_DEVICE_ARRAY", true, false);
  checkDoocsProperty<D_bytearray>("//CHAR/TO_DEVICE_ARRAY");
  checkDoocsProperty<D_int>("//CHAR/DATA_TYPE_CONSTANT", true, false);
  CHECK_WITH_TIMEOUT(DoocsServerTestHelper::doocsGet<int>("//CHAR/DATA_TYPE_CONSTANT") == -1);
  checkDoocsProperty<D_int>("//CHAR/FROM_DEVICE_SCALAR", true, false);
  checkDoocsProperty<D_int>("//CHAR/TO_DEVICE_SCALAR");
}
