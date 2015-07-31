#include <sstream>

#include <boost/test/included/unit_test.hpp>
//using namespace boost::unit_test_framework;
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::framework::master_test_suite;

#include "DoocsProcessScalar.h"
#include "DoocsPVFactory.h"
#include "emptyServerFunctions.h"

#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/ControlSystemProcessScalar.h>

namespace mtca4u {

  /// a class to access the internal lists of the DoocsPVManager
  class TestableDoocsPVManager: public DoocsPVManager{
  public:
    /// empty constructor, just pass the argument to the parent class
    TestableDoocsPVManager(boost::shared_ptr<ControlSystemPVManager> pvManager)
      : DoocsPVManager( pvManager ) {}

    std::list< ControlSystemProcessVariable::SharedPtr > const & getToDeviceProcessVariables(){
      return _toDeviceProcessVariables;
    }

    //std::list< ControlSystemProcessVariable::SharedPtr > const & getFromDeviceProcessVariables(){
    //  return _fromDeviceProcessVariables;
    //    }
  };

  /** The test class for the DoocsProcessScalar.
   *  It is templated to be tested with all data types.
   */
  template<class T>
  class DoocsProcessScalarTest {
  public:
    DoocsProcessScalarTest();
    // constructors are tested by the factory. No need to repeat that here
    void testAssignment();
    void testSetters();
    void testConversionOperator();

  private:
    // check that the synchronisation works: The last member in the DoocsPVManager's
    // to device list must be a pointer to the processT instance.
    void checkLastListMemberIsThisProcessT();

    boost::shared_ptr< ControlSystemProcessScalar<T> > _processT;
    boost::shared_ptr<TestableDoocsPVManager> _pvManager;
  };

  /** The boost test suite which executes the DoocsProcessScalarTest.
   */
  template<class T>
  class DoocsProcessScalarTestSuite: public test_suite {
  public:
    DoocsProcessScalarTestSuite() :
        test_suite("DoocsProcessScalar test suite") {
      boost::shared_ptr<DoocsProcessScalarTest<T> > processScalarTest(
          new DoocsProcessScalarTest<T>);

      add(
          BOOST_CLASS_TEST_CASE(&DoocsProcessScalarTest<T>::testAssignment,
              processScalarTest));
      add(
          BOOST_CLASS_TEST_CASE(&DoocsProcessScalarTest<T>::testSetters,
              processScalarTest));
      add(
          BOOST_CLASS_TEST_CASE(
              &DoocsProcessScalarTest<T>::testConversionOperator,
              processScalarTest));
    }
  };

  template<class T>
  DoocsProcessScalarTest<T>::DoocsProcessScalarTest(){
    //the regular manageres are that standard mtca4u control system types
    boost::shared_ptr<DoocsPVFactory> factory( new DoocsPVFactory( NULL ) );
    std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > regularPvManagers =
      createPVManager(factory);
    
    // we are using a doocs manager type
    _pvManager.reset( new TestableDoocsPVManager( regularPvManagers.first ) );
    factory->setDoocsPVManager( _pvManager );

    regularPvManagers.second->createProcessScalar<T>( "CLASS_WIDE_T" );
    regularPvManagers.second->createProcessScalar<T>( "SECOND_T" );
   
    _processT = regularPvManagers.first->getProcessScalar<T>( "CLASS_WIDE_T" );
    assert(_processT);
  }

  template<class T>
  void DoocsProcessScalarTest<T>::testAssignment() {
    // Note: Will be removed in the next version of the API, no need to write tests

    // assignment of T and automatic conversion (in the == test)
    *_processT = 3;
    BOOST_CHECK(*_processT == 3);
    // check that the modified reaches the manager
    BOOST_CHECK( _pvManager->getToDeviceProcessVariables().back().get() == 
		 _processT.get() );
    
    boost::shared_ptr< ControlSystemProcessScalar<T> > secondT = 
      _pvManager->getProcessScalar<T>( "SECOND_T" );
    *secondT = 2;
    *_processT = *secondT;
    BOOST_CHECK(*_processT == 2);
  }

  template<class T>
  void DoocsProcessScalarTest<T>::testSetters() {
    // will be removed on the next version of the API
  }

  template<class T>
  void DoocsProcessScalarTest<T>::testConversionOperator() {
    // will be removed in the next version of the API
  }

}  //namespace mtca4u

test_suite*
init_unit_test_suite(int /*argc*/, char* /*argv*/[]) {
  std::cout << "this is init_unit_test_suite" << std::endl;
  master_test_suite().p_name.value =
      "DoocsProcessScalar test suite";

  std::cout << "name ok" << std::endl;
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<int32_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<uint32_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<int16_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<uint16_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<int8_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<uint8_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<double>);
  master_test_suite().add(
      new mtca4u::DoocsProcessScalarTestSuite<float>);

  std::cout << "suites are there" << std::endl;

  return NULL;
}

