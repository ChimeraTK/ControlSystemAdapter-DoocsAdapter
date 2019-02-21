// Define a name for the test module.
#define BOOST_TEST_MODULE BasenameFromAddressTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include "basenameFromAddress.h"

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

BOOST_AUTO_TEST_SUITE(BasenameFromAddressTestSuite)

BOOST_AUTO_TEST_CASE(testSpitting) {
  BOOST_CHECK(basenameFromAddress("//LOCATION/PROPERTY.NAME") ==
              "PROPERTY.NAME");
  BOOST_CHECK(basenameFromAddress("PROPERTY.NAME") == "PROPERTY.NAME");
  BOOST_CHECK(basenameFromAddress("/PROPERTY.NAME") == "PROPERTY.NAME");
  BOOST_CHECK(basenameFromAddress("//PROPERTY.NAME/") == "");
}

BOOST_AUTO_TEST_SUITE_END()
