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

    std::list< ControlSystemProcessVariable::SharedPtr > const & getToDeviceProcessVariables() const{
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

    /// The assigment test function which is given to the boost framework.
    /// Unfortunately it can only have one template parameter (T in this case).
    /// There is no general implementation but only specialisations which bind
    /// this function to the correct testAssigmentImpl template combination.
    void testAssignment();

    /// For the real test we need a function with two template parameters because
    /// the DoocsProcessScalar if of type <T, DOOCS_T>
    template<class DOOCS_T> void testAssignmentImpl();

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

  template<> void DoocsProcessScalarTest<int32_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }

  template<> void DoocsProcessScalarTest<uint32_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }

  template<> void DoocsProcessScalarTest<int16_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }

  template<> void DoocsProcessScalarTest<uint16_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }

  template<> void DoocsProcessScalarTest<int8_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }

  template<> void DoocsProcessScalarTest<uint8_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }

  template<> void DoocsProcessScalarTest<double>::testAssignment() {
    testAssignmentImpl<D_double>();
  }

  template<> void DoocsProcessScalarTest<float>::testAssignment() {
    testAssignmentImpl<D_float>();
  }

  template<class T> template<class DOOCS_T>
  void DoocsProcessScalarTest<T>::testAssignmentImpl() {

    // assignment of T and automatic conversion (in the == test)
    boost::shared_ptr< DoocsProcessScalar< T, DOOCS_T > > doocsProcessT =
      boost::dynamic_pointer_cast< DoocsProcessScalar< T, DOOCS_T > >( _processT );

    boost::shared_ptr< DoocsProcessScalar< T, DOOCS_T > > doocsSecondT =
      boost::dynamic_pointer_cast< DoocsProcessScalar< T, DOOCS_T > >( _pvManager->getProcessScalar<T>( "SECOND_T" ) );

    *doocsProcessT = 3;
    BOOST_CHECK(*doocsProcessT == 3);

    // check that the modified reaches the manager
    BOOST_CHECK( _pvManager->getToDeviceProcessVariables().back() == 
		 _processT );
    
    *doocsSecondT = 2;
    *doocsProcessT = *doocsSecondT;
    BOOST_CHECK(*doocsProcessT == 2);
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

