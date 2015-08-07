#include <sstream>

#include <boost/test/included/unit_test.hpp>
//using namespace boost::unit_test_framework;
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::framework::master_test_suite;

#include <boost/scoped_ptr.hpp>

#include "DoocsProcessScalar.h"
#include "DoocsPVFactory.h"
#include "emptyServerFunctions.h"

#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/ControlSystemProcessScalar.h>
#include <ControlSystemAdapter/DeviceProcessScalar.h>

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
  };

  template <class T >
  struct TestDeviceCallable {
    boost::shared_ptr<DevicePVManager> pvManager;
    boost::shared_ptr< DeviceProcessScalar<T> > readMe;
    boost::shared_ptr< DeviceProcessScalar<T> > writeMe;
    DeviceProcessScalar<int8_t>::SharedPtr stopDeviceThread;

    TestDeviceCallable(boost::shared_ptr<DevicePVManager> & pvManager_) : pvManager( pvManager_){
      readMe = pvManager->createProcessScalar<T>("readMe");
      writeMe = pvManager->createProcessScalar<T>("writeMe");
      stopDeviceThread = pvManager->createProcessScalar<int8_t>("stopDeviceThread");
    }

    void operator()() {
      //      DeviceProcessArray<typename T>::SharedPtr readMeArray = pvManager->getProcessArray<T>("readMeArray");
      //      DeviceProcessArray<float>::SharedPtr writeMeArray = pvManager->getProcessArray< float>("writeMeArray");

      stopDeviceThread->set(0);

      readMe->set(0);
      writeMe->set(0);
      //      readMeArray->set(vector<float>(10, 0.0));
      //      writeMeArray->set(vector<float>(10, 0.0));

      while (stopDeviceThread->get() == 0) {
        pvManager->processSynchronization(10000);
        *readMe = *writeMe;
        //*readMeArray = *writeMeArray;
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
      }
    }
  };

  /** The test class for the DoocsProcessVariables.
   *  It is templated to be tested with all data types.
   */
  template<class T>
  class DoocsProcessVariablesTest {
  public:
    DoocsProcessVariablesTest();
    // constructors are tested by the factory. No need to repeat that here

    /// The assigment test function which is given to the boost framework.
    /// Unfortunately it can only have one template parameter (T in this case).
    /// There is no general implementation but only specialisations which bind
    /// this function to the correct testAssigmentImpl template combination.
    void testAssignment();

    /// For the real test we need a function with two template parameters because
    /// the DoocsProcessVariables if of type <T, DOOCS_T>
    template<class DOOCS_T> void testAssignmentImpl();

    void testSynchronisation();
    template<class DOOCS_T> void testSynchronisationImpl();

  private:
    boost::shared_ptr< ControlSystemProcessScalar<T> > _processT;
    boost::shared_ptr<TestableDoocsPVManager> _pvManager;
    boost::scoped_ptr< TestDeviceCallable<T> > _testDeviceCallable;
  };

  /** The boost test suite which executes the DoocsProcessVariablesTest.
   */
  template<class T>
  class DoocsProcessVariablesTestSuite: public test_suite {
  public:
    DoocsProcessVariablesTestSuite() :
        test_suite("DoocsProcessVariables test suite") {
      boost::shared_ptr<DoocsProcessVariablesTest<T> > processScalarTest(
          new DoocsProcessVariablesTest<T>);

      add( BOOST_CLASS_TEST_CASE(&DoocsProcessVariablesTest<T>::testAssignment,
              processScalarTest));
      add( BOOST_CLASS_TEST_CASE(&DoocsProcessVariablesTest<T>::testSynchronisation,
              processScalarTest));
    }
  };

  template<class T>
  DoocsProcessVariablesTest<T>::DoocsProcessVariablesTest(){
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
    _testDeviceCallable.reset( new TestDeviceCallable<T>(regularPvManagers.second) );
  }

  template<> void DoocsProcessVariablesTest<int32_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<uint32_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<int16_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<uint16_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<int8_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<uint8_t>::testAssignment() {
    testAssignmentImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<double>::testAssignment() {
    testAssignmentImpl<D_double>();
  }
  template<> void DoocsProcessVariablesTest<float>::testAssignment() {
    testAssignmentImpl<D_float>();
  }

  template<class T> template<class DOOCS_T>
  void DoocsProcessVariablesTest<T>::testAssignmentImpl() {

    // assignment of T and automatic conversion (in the == test)
    boost::shared_ptr< DoocsProcessScalar< T, DOOCS_T > > doocsProcessT =
      boost::dynamic_pointer_cast< DoocsProcessScalar< T, DOOCS_T > >( _processT );

    boost::shared_ptr< DoocsProcessScalar< T, DOOCS_T > > doocsSecondT =
      boost::dynamic_pointer_cast< DoocsProcessScalar< T, DOOCS_T > >(
        _pvManager->getProcessScalar<T>( "SECOND_T" ) );

    *doocsProcessT = static_cast<T>(3.7);
    BOOST_CHECK(*doocsProcessT == static_cast<T>(3.7) );

    // check that the modified reaches the manager
    BOOST_CHECK( _pvManager->getToDeviceProcessVariables().back() == 
		 _processT );
    
    *doocsSecondT = 2;
    *doocsProcessT = *doocsSecondT;
    BOOST_CHECK(*doocsProcessT == 2);
  }

  template<> void DoocsProcessVariablesTest<int32_t>::testSynchronisation() {
    testSynchronisationImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<uint32_t>::testSynchronisation() {
    testSynchronisationImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<int16_t>::testSynchronisation() {
    testSynchronisationImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<uint16_t>::testSynchronisation() {
    testSynchronisationImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<int8_t>::testSynchronisation() {
    testSynchronisationImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<uint8_t>::testSynchronisation() {
    testSynchronisationImpl<D_int>();
  }
  template<> void DoocsProcessVariablesTest<double>::testSynchronisation() {
    testSynchronisationImpl<D_double>();
  }
  template<> void DoocsProcessVariablesTest<float>::testSynchronisation() {
    testSynchronisationImpl<D_float>();
  }

  template<class T> template<class DOOCS_T>
  void DoocsProcessVariablesTest<T>::testSynchronisationImpl(){
    // when the thread starts, it resets the values to 0 in the callable
     boost::thread deviceThread(*_testDeviceCallable);

     boost::shared_ptr< DoocsProcessScalar< T, DOOCS_T > > writeMe =
       boost::dynamic_pointer_cast< DoocsProcessScalar< T, DOOCS_T > >(
	 _pvManager->getProcessScalar<T>( "writeMe" ) );

     boost::shared_ptr< DoocsProcessScalar< T, DOOCS_T > > readMe =
       boost::dynamic_pointer_cast< DoocsProcessScalar< T, DOOCS_T > >(
         _pvManager->getProcessScalar<T>( "readMe" ) );
 
     *writeMe = 33;

     // check that the syncronisation list is ok.
     BOOST_CHECK( _pvManager->getToDeviceProcessVariables().back() == 
		 writeMe );

     // synchronise twice: First one gets it into the device, which sets readMe after the sync.
     // The second one get readMe out.
     _pvManager->synchronize();
     _pvManager->synchronize();
     
     // After synchronisation the list has to be empty
     BOOST_CHECK( _pvManager->getToDeviceProcessVariables().empty() );

     std::stringstream message;
     message << "readMe is " << static_cast<double>(*readMe) << std::endl;
     BOOST_CHECK_MESSAGE( *readMe == 33 , message.str());

     // tell the tread to end and wait for this to happen
     boost::dynamic_pointer_cast< DoocsProcessScalar< int8_t, D_int > >(
       _pvManager->getProcessScalar<int8_t>( "stopDeviceThread" ) )->set_value(1);
     
     BOOST_CHECK( _pvManager->getToDeviceProcessVariables().size() == 1 );
     BOOST_CHECK( _pvManager->getToDeviceProcessVariables().back() == 
		  _pvManager->getProcessScalar<int8_t>( "stopDeviceThread" ) );

     _pvManager->synchronize();

     BOOST_CHECK( _pvManager->getToDeviceProcessVariables().empty() );
    
     deviceThread.join();
  }

}  //namespace mtca4u

test_suite*
init_unit_test_suite(int /*argc*/, char* /*argv*/[]) {
  std::cout << "this is init_unit_test_suite" << std::endl;
  master_test_suite().p_name.value =
      "DoocsProcessVariables test suite";

  std::cout << "name ok" << std::endl;
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<int32_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<uint32_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<int16_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<uint16_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<int8_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<uint8_t>);
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<double>);
  master_test_suite().add(
      new mtca4u::DoocsProcessVariablesTestSuite<float>);

  std::cout << "suites are there" << std::endl;

  return NULL;
}

