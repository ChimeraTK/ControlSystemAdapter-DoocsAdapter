#include <boost/test/included/unit_test.hpp>

//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>

#include <boost/filesystem.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>

ReferenceTestApplication referenceTestApplication("serverTestRetypeProperties");

// declare that we have some thing like a doocs server. is is linked from the
// doocs lib, but there is no header.
extern int eq_server(int, char**);

//#include <limits>
//#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

// use boost meta-programming to use test case templates
// The list of types is an mpl type
// typedef boost::mpl::list<int32_t, uint32_t,
//			 int16_t, uint16_t,
//			 int8_t, uint8_t,
//			 float, double> simple_test_types;

/// Check that all expected variables are there.
void testVariableExistence() {
  checkDoocsProperty<D_int>("//TO_BYTE/DOUBLE.CONSTANT", true, false);
  checkDoocsProperty<D_int>("//TO_SHORT/FLOAT.CONSTANT", true, false);
  checkDoocsProperty<D_int>("//TO_INT/SHORT.CONSTANT", true, false);
  checkDoocsProperty<D_longarray>("//TO_LONG/UCHAR.CONSTANT", true, false);
  checkDoocsProperty<D_float>("//TO_FLOAT/INT.CONSTANT", true, false);
  checkDoocsProperty<D_double>("//TO_DOUBLE/USHORT.CONSTANT", true, false);
}

// due to the doocs server thread you can only have one test suite
class serverTestRetypePropertiesTestSuite : public test_suite {
 public:
  serverTestRetypePropertiesTestSuite(int argc, char* argv[]) : test_suite("serverTestRetypeProperties test suite") {
    // create DOOCS thread
    doocsServerThread = std::thread(eq_server, argc, argv);
    // wait for doocs to start up before detaching the thread and continuing
    ChimeraTK::DoocsAdapter::waitUntilInitialised();
    add(BOOST_TEST_CASE(&testVariableExistence));
  }

  ~serverTestRetypePropertiesTestSuite() {
    eq_exit();
    doocsServerThread.join();
  }

 protected:
  std::thread doocsServerThread;
};

test_suite* init_unit_test_suite(int argc, char* argv[]) {
  framework::master_test_suite().p_name.value = "serverTestRetypeProperties server test suite";
  return new serverTestRetypePropertiesTestSuite(argc, argv);
}
