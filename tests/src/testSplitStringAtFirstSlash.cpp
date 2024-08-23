// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

// Define a name for the test module.
#define BOOST_TEST_MODULE SplitStringAtFirstSlashTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
// boost unit_test needs to be included before serverBasedTestTools.h
#include "splitStringAtFirstSlash.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

BOOST_AUTO_TEST_SUITE(SplitTestSuite)

BOOST_AUTO_TEST_CASE(testSpitting) {
  auto splitResult = splitStringAtFirstSlash("stringWithout.a.slash");
  BOOST_CHECK(splitResult.first.empty());
  BOOST_CHECK(splitResult.second == "stringWithout.a.slash");

  splitResult = splitStringAtFirstSlash("/stringStartingWith.slash");
  BOOST_CHECK(splitResult.first.empty());
  BOOST_CHECK(splitResult.second == "stringStartingWith.slash");

  splitResult = splitStringAtFirstSlash("stringEndingWith.slash/");
  BOOST_CHECK(splitResult.first == "stringEndingWith.slash");
  BOOST_CHECK(splitResult.second.empty());

  splitResult = splitStringAtFirstSlash("string/With.slash");
  BOOST_CHECK(splitResult.first == "string");
  BOOST_CHECK(splitResult.second == "With.slash");

  splitResult = splitStringAtFirstSlash("string/With/twoSlashes");
  BOOST_CHECK(splitResult.first == "string");
  BOOST_CHECK(splitResult.second == "With.twoSlashes"); // second slash gets replaced with dot
}

BOOST_AUTO_TEST_SUITE_END()
