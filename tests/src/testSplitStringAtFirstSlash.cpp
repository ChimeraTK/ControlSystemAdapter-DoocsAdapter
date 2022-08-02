// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

// Define a name for the test module.
#define BOOST_TEST_MODULE SplitStringAtFirstSlashTest
// Only after defining the name include the unit test header.
#include "emptyServerFunctions.h"
#include "splitStringAtFirstSlash.h"

#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

BOOST_AUTO_TEST_SUITE(SplitTestSuite)

BOOST_AUTO_TEST_CASE(testSpitting) {
  auto splitResult = splitStringAtFirstSlash("stringWithout.a.slash");
  BOOST_CHECK(splitResult.first == "");
  BOOST_CHECK(splitResult.second == "stringWithout.a.slash");

  splitResult = splitStringAtFirstSlash("/stringStartingWith.slash");
  BOOST_CHECK(splitResult.first == "");
  BOOST_CHECK(splitResult.second == "stringStartingWith.slash");

  splitResult = splitStringAtFirstSlash("stringEndingWith.slash/");
  BOOST_CHECK(splitResult.first == "stringEndingWith.slash");
  BOOST_CHECK(splitResult.second == "");

  splitResult = splitStringAtFirstSlash("string/With.slash");
  BOOST_CHECK(splitResult.first == "string");
  BOOST_CHECK(splitResult.second == "With.slash");

  splitResult = splitStringAtFirstSlash("string/With/twoSlashes");
  BOOST_CHECK(splitResult.first == "string");
  BOOST_CHECK(splitResult.second == "With.twoSlashes"); // second slash gets replaced with dot
}

BOOST_AUTO_TEST_SUITE_END()
