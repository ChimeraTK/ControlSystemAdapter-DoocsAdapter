#include <boost/test/included/unit_test.hpp>

#include <boost/filesystem.hpp>

// #include <boost/test/unit_test.hpp>

#include "DoocsAdapter.h"
#include "serverBasedTestTools.h"
#include <ChimeraTK/ControlSystemAdapter/Testing/ReferenceTestApplication.h>
#include <doocs-server-test-helper/doocsServerTestHelper.h>
#include <thread>

ReferenceTestApplication referenceTestApplication("serverTestVariableMapperWithLocationAndCode");

// declare that we have some thing like a doocs server. is is linked from the
// doocs lib, but there is no header.
extern int eq_server(int, char**);

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

void testCodeIsSetCorrectly() {

  EqFct* eq = find_device("CREATED");
  BOOST_CHECK_EQUAL( eq->fct_code(), 10 );
  
  eq = find_device("NEW");
  BOOST_CHECK_EQUAL( eq->fct_code(), 12 );
  
  eq = find_device("DOUBLE");
  BOOST_CHECK_EQUAL( eq->fct_code(), 10 );
  
  eq = find_device("FLOAT");
  BOOST_CHECK_EQUAL( eq->fct_code(), 11 );
    
}

// due to the doocs server thread you can only have one test suite
class serverTestVariableMapperWithLocationAndCodeTestSuite : public test_suite {
 public:
  serverTestVariableMapperWithLocationAndCodeTestSuite(int argc, char* argv[]) : test_suite("serverTestVariableMapperWithLocationAndCode test suite") {
    // create DOOCS thread
    doocsServerThread = std::thread(eq_server, argc, argv);
    // wait for doocs to start up before detaching the thread and continuing
    ChimeraTK::DoocsAdapter::waitUntilInitialised();
    add(BOOST_TEST_CASE(&testCodeIsSetCorrectly));
  }
  /**
 * @brief For compatibility with older DOOCS versions declare our own eq_exit
 *
 * Can be removed once a new doocs server version is released.
 */
  void eq_exit() {
    auto nativeHandle = doocsServerThread.native_handle();
    if(nativeHandle != 0) pthread_kill(nativeHandle, SIGTERM);
  }

  ~serverTestVariableMapperWithLocationAndCodeTestSuite() {
    eq_exit();
    doocsServerThread.join();
  }

 protected:
  std::thread doocsServerThread;
};

test_suite* init_unit_test_suite(int argc, char* argv[]) {
  framework::master_test_suite().p_name.value = "serverTestVariableMapperWithLocationAndCode server test suite";
  return new serverTestVariableMapperWithLocationAndCodeTestSuite(argc, argv);
}
