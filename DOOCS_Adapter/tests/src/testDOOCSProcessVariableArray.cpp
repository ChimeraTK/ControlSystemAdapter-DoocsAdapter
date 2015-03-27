#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_tools.hpp>
using namespace boost::unit_test_framework;


#include "dice.hpp"


#include <eq_fct.h>

#include "m4uD_array.hpp"

#include "DOOCSProcessVariableArray.hpp"



/** A class that exists just to create a different type of ProcessArray for testing.
 */
template <typename T, typename M4U_DOOCSARR_T>
class AssignTestProcessArray: public mtca4u::DOOCSProcessVariableArray<T, M4U_DOOCSARR_T>
{
public:
    AssignTestProcessArray(boost::shared_ptr< M4U_DOOCSARR_T > _m4uD_array_T, size_t arraySize) : mtca4u::DOOCSProcessVariableArray<T, M4U_DOOCSARR_T> (_m4uD_array_T, arraySize) {}
};






////////////////////////////////////////////////////////////////---<>
                                                              //
template <typename T>
class DOOCSProcessVariableArrayTest
{

private:

    size_t const N_ELEMENTS;
    T      const G_FILL_VALUE; // for gCALLBACK
    
    std::vector<T> referenceVector;
    std::vector<T> toolarge_referenceVector;
    
    unsigned int __setCallbackCounter;
    unsigned int __getCallbackCounter;
    
    boost::shared_ptr< mtca4u::m4uD_array<T> >   darray;            mtca4u::DOOCSProcessVariableArray<T, mtca4u::m4uD_array<T> >  doocs_process_array;
    boost::shared_ptr< mtca4u::m4uD_array<T> >   emptydarray;       mtca4u::DOOCSProcessVariableArray<T, mtca4u::m4uD_array<T> >  empty_doocs_process_array;
    boost::shared_ptr< mtca4u::m4uD_array<T> >   refdarray;         mtca4u::DOOCSProcessVariableArray<T, mtca4u::m4uD_array<T> >  reference_doocs_process_array;
    //~ boost::shared_ptr< mtca4u::m4uD_array<T> >   atrefdarray;                  AssignTestProcessArray<T, mtca4u::m4uD_array<T> >  assign_test_reference_process_array;        // iterators required for this to work
    
    boost::shared_ptr< mtca4u::m4uD_array<T> >   toolarge1darray;   mtca4u::DOOCSProcessVariableArray<T, mtca4u::m4uD_array<T> >  toolarge_ref_doocs_process_array;
    boost::shared_ptr< mtca4u::m4uD_array<T> >   toolarge2darray;              AssignTestProcessArray<T, mtca4u::m4uD_array<T> >  toolarge_assign_test_ref_process_array;
    
    mtca4u::ProcessArray<T> & _process_array;
    mtca4u::ProcessArray<T> & _reference_process_array;


    void __resetCallbackCounters()
    {
        __setCallbackCounter = 0;
        __getCallbackCounter = 0;
    }


public:

    DOOCSProcessVariableArrayTest() :    N_ELEMENTS               (3)
                                        ,G_FILL_VALUE             (50)
                                        //~ ,SOME_NUMBER(42)
                                        
                                        ,referenceVector          (N_ELEMENTS, 6)
                                        ,toolarge_referenceVector (N_ELEMENTS+1, 6)
                                        
                                        ,__setCallbackCounter     (0)
                                        ,__getCallbackCounter     (0)
                                        
                                        ,darray          ( new mtca4u::m4uD_array<T> ( NULL, N_ELEMENTS, NULL ) )      ,doocs_process_array                    (darray          , N_ELEMENTS)
                                        ,emptydarray     ( new mtca4u::m4uD_array<T> ( NULL,          0, NULL ) )      ,empty_doocs_process_array              (emptydarray     ,          0)
                                        ,refdarray       ( new mtca4u::m4uD_array<T> ( NULL, N_ELEMENTS, NULL ) )      ,reference_doocs_process_array          (refdarray       , N_ELEMENTS)
                                        //~ ,atrefdarray     ( new mtca4u::m4uD_array<T> ( NULL, N_ELEMENTS, NULL ) )      ,assign_test_reference_process_array    (atrefdarray     , N_ELEMENTS)        // iterators required for this to work
                                        
                                        ,toolarge1darray ( new mtca4u::m4uD_array<T> ( NULL, N_ELEMENTS+1, NULL ) )    ,toolarge_ref_doocs_process_array       (toolarge1darray , N_ELEMENTS+1)
                                        ,toolarge2darray ( new mtca4u::m4uD_array<T> ( NULL, N_ELEMENTS+1, NULL ) )    ,toolarge_assign_test_ref_process_array (toolarge2darray , N_ELEMENTS+1)
                                        
                                        ,_process_array(doocs_process_array)
                                        ,_reference_process_array(reference_doocs_process_array)
    {
        _process_array.setOnSetCallbackFunction( boost::bind( &DOOCSProcessVariableArrayTest::setCBfun, this, _1) );
        _process_array.setOnGetCallbackFunction( boost::bind( &DOOCSProcessVariableArrayTest::getCBfun, this, _1) );
    }
    

    // sCALLBACK
    void setCBfun(mtca4u::ProcessArray<T> const & ){
        ++__setCallbackCounter;
    }
    
    // gCALLBACK
    void getCBfun(mtca4u::ProcessArray<T> & toBeFilled ){
        toBeFilled.fill(G_FILL_VALUE);
        ++__getCallbackCounter;  
    }

    
    void testSizeEmpty();
    void testFill();
    void testSetWithoutCallback();
    void testSet();
    void testGetWithoutCallback();
    void testGet();
    void test_fillvector();
    void testAssignment();

};
                                                              //
////////////////////////////////////////////////////////////////---<>




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                                                           //
template <typename T>
void DOOCSProcessVariableArrayTest<T>::testSizeEmpty()
{
    BOOST_CHECK_EQUAL ( doocs_process_array.size() , N_ELEMENTS );
    BOOST_CHECK_EQUAL ( doocs_process_array.empty(), false      );

    BOOST_CHECK_EQUAL ( empty_doocs_process_array.size() , 0    );
    BOOST_CHECK_EQUAL ( empty_doocs_process_array.empty(), true );


    BOOST_CHECK_EQUAL ( darray->length()      , N_ELEMENTS );
    BOOST_CHECK_EQUAL ( emptydarray->length() , 0          );
}


template <typename T>
void DOOCSProcessVariableArrayTest<T>::testFill()
{
    doocs_process_array.fill(30);
    
    for(int i=0; i<darray->length(); ++i)
        BOOST_CHECK_EQUAL(darray->read_spectrum (i), 30);
}
                                                                                                           //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////




template <typename T>
void DOOCSProcessVariableArrayTest<T>::testSetWithoutCallback()
{
    __resetCallbackCounters();
    
    // fill something known so we can check that it changed
    _process_array.fill(5);
    
    _process_array.setWithoutCallback(referenceVector);
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
      BOOST_CHECK_EQUAL(referenceVector[i], darray->read_spectrum ((int)i));
    }


    reference_doocs_process_array.fill(7);
    
    _process_array.setWithoutCallback(reference_doocs_process_array);
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
      BOOST_CHECK_EQUAL(
          refdarray->read_spectrum ((int)i),
          darray   ->read_spectrum ((int)i)
      );
    }

    
    _reference_process_array.fill(8);
    _process_array.setWithoutCallback(_reference_process_array);
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
      BOOST_CHECK_EQUAL(
          refdarray->read_spectrum ((int)i),
          darray   ->read_spectrum ((int)i)
      );
    }
    
    //~ assign_test_reference_process_array.fill(9);                    // iterators required        FIXME: but then not intended
    //~ doocs_process_array.setWithoutCallback(assign_test_reference_process_array);
    //~ for (size_t i=0; i<N_ELEMENTS; ++i)
    //~ {
      //~ BOOST_CHECK_EQUAL(
          //~ atrefdarray->read_spectrum ((int)i),
          //~ darray     ->read_spectrum ((int)i)
      //~ );
    //~ }
    

    BOOST_CHECK_THROW( _process_array.setWithoutCallback( toolarge_assign_test_ref_process_array ), std::out_of_range );
    
    BOOST_CHECK_THROW( _process_array.setWithoutCallback( toolarge_ref_doocs_process_array ), std::out_of_range );
    BOOST_CHECK_THROW( doocs_process_array.setWithoutCallback( toolarge_ref_doocs_process_array ), std::out_of_range );
    
    BOOST_CHECK_THROW( _process_array.setWithoutCallback( toolarge_referenceVector ), std::out_of_range );


    BOOST_CHECK_EQUAL( __getCallbackCounter, 0 );
    BOOST_CHECK_EQUAL( __setCallbackCounter, 0 );
}


template <typename T>
void DOOCSProcessVariableArrayTest<T>::testSet()
{
    __resetCallbackCounters();

    // fill something known so we can check that it changed
    _process_array.fill(5);
    
    _process_array.set(referenceVector);
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL(referenceVector[i], darray->read_spectrum ((int)i));
    }
    BOOST_CHECK_EQUAL( __setCallbackCounter, 1 );


    reference_doocs_process_array.fill(7);
    
    _process_array.set(reference_doocs_process_array);
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
      BOOST_CHECK_EQUAL(
          refdarray->read_spectrum ((int)i),
          darray   ->read_spectrum ((int)i)
      );
    }
    BOOST_CHECK_EQUAL( __setCallbackCounter, 2 );


  // unregister the callback function...
    _process_array.clearOnSetCallbackFunction();
    
    
    // ... and try again
    reference_doocs_process_array.fill(8);
    
    _process_array.set(reference_doocs_process_array);
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
      BOOST_CHECK_EQUAL(
          refdarray->read_spectrum ((int)i),
          darray   ->read_spectrum ((int)i)
      );
    }
    BOOST_CHECK_EQUAL( __setCallbackCounter, 2 );


    _process_array.set(referenceVector);    // 6'es again
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL(referenceVector[i], darray->read_spectrum ((int)i));
    }
    BOOST_CHECK_EQUAL( __setCallbackCounter, 2 );


    // toolarges
    BOOST_CHECK_THROW( _process_array.set( toolarge_ref_doocs_process_array ), std::out_of_range );
    BOOST_CHECK_THROW( _process_array.setWithoutCallback( toolarge_referenceVector ), std::out_of_range );
    
    
    BOOST_CHECK_EQUAL( __setCallbackCounter, 2 );
    BOOST_CHECK_EQUAL( __getCallbackCounter, 0 );
}



template <typename T>
void DOOCSProcessVariableArrayTest<T>::testGetWithoutCallback()
{
    __resetCallbackCounters();
    
    // fill something known so we can check that it changed
    _process_array.fill(5);


    std::vector<T> const & result_of_get = doocs_process_array.getWithoutCallback();

    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL( result_of_get[i], darray->read_spectrum ((int)i));
    }


    //~ _process_array.fill(4);


    //~ result_of_get = _process_array.getWithoutCallback();        FIXME: class mtca4u::ProcessArray<T>’ has no member named ‘getWithoutCallback'
//~ 
    //~ for (size_t i=0; i<N_ELEMENTS; ++i)
    //~ {
        //~ BOOST_CHECK_EQUAL( result_of_get[i], darray->read_spectrum ((int)i));
    //~ }


    // FIXME: assign_test_reference_process_array?

    BOOST_CHECK_EQUAL( __setCallbackCounter, 0 );
    BOOST_CHECK_EQUAL( __getCallbackCounter, 0 );
}


template <typename T>
void DOOCSProcessVariableArrayTest<T>::testGet()
{
    __resetCallbackCounters();
    
    // fill something known so we can check that it changed
    _process_array.fill(5);


    std::vector<T> const & result_of_get1 = doocs_process_array.get();
    BOOST_CHECK_EQUAL( __getCallbackCounter, 1 );
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL( G_FILL_VALUE, darray->read_spectrum ((int)i) );
        BOOST_CHECK_EQUAL( G_FILL_VALUE, result_of_get1[i] );
    }

    
    // clear the getter callback function
    _process_array.clearOnGetCallbackFunction();
    _process_array.fill(5);


    std::vector<T> const & result_of_get2 = doocs_process_array.get();
    BOOST_CHECK_EQUAL( __getCallbackCounter, 1 );
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL( 5, darray->read_spectrum ((int)i) );
        BOOST_CHECK_EQUAL( 5, result_of_get2[i] );
    }
}


template <typename T>
void DOOCSProcessVariableArrayTest<T>::test_fillvector()
{
    __resetCallbackCounters();
    
    _process_array = referenceVector;


    std::vector<T> v(N_ELEMENTS);
    doocs_process_array.fillVector(v);

    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL( v[i], referenceVector[i] );
    }


    // toolarges
    BOOST_CHECK_THROW( doocs_process_array.fillVector( toolarge_referenceVector ), std::out_of_range );


    BOOST_CHECK_EQUAL( __setCallbackCounter, 0 );
    BOOST_CHECK_EQUAL( __getCallbackCounter, 0 );
}


template <typename T>
void DOOCSProcessVariableArrayTest<T>::testAssignment()
{
    __resetCallbackCounters();
    
    // assign from a vector ...
    _process_array = referenceVector;
    // ... and check 
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL(referenceVector[i], darray->read_spectrum ((int)i));
    }

    // assign from another DOOCSProcessVariableArray (1)
    // // prepare some garbage
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        referenceVector[i] = *die++;    // random(1,6)
    }
    reference_doocs_process_array = referenceVector;
    // // assign ...
    doocs_process_array = reference_doocs_process_array;
    // // ... and check 
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL(referenceVector[i], darray->read_spectrum ((int)i));
    }


    // test self assignment to check if code coverage goes up
    doocs_process_array = doocs_process_array;

  
    //~ // assign from another DOOCSProcessVariableArray (2)
    //~ // // re-pollute reference_doocs_process_array
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        referenceVector[i] = *die++;    // random(1,6)
    }
    reference_doocs_process_array = referenceVector;
    // // assign ...
    _process_array = reference_doocs_process_array;
    // // ... and check 
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL(referenceVector[i], darray->read_spectrum ((int)i));
    }


  //~ // and yet another test, this time with a different implementation of ProcessArray            // again, iterators required (probably)        FIXME: but then not intended
  //~ AssignTestProcessArray<T> assignTestProcessArray(N_ELEMENTS);
  //~ assignTestProcessArray.setWithoutCallback(_referenceVector); // FIXME: why does the assignment operator not work?
  //~ // we need to reverse again for the rest 
  //~ std::sort(  assignTestProcessArray.rbegin(),  assignTestProcessArray.rend() );
  //~ _process_array = assignTestProcessArray;
  //~ BOOST_CHECK( std::equal( _process_array.begin(),  _process_array.end(), _referenceVector.rbegin()) );
  //~ 


    BOOST_CHECK_THROW( _process_array      = toolarge_assign_test_ref_process_array, std::out_of_range );
    
    BOOST_CHECK_THROW( _process_array      = toolarge_ref_doocs_process_array,       std::out_of_range );
    BOOST_CHECK_THROW( doocs_process_array = toolarge_ref_doocs_process_array,       std::out_of_range );
    
    BOOST_CHECK_THROW( _process_array      = toolarge_referenceVector,               std::out_of_range );


    BOOST_CHECK_EQUAL( __getCallbackCounter, 0 );
    BOOST_CHECK_EQUAL( __setCallbackCounter, 0 );
}


// FIXME: inter-PA behavior tests required



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>
                                                                                                                                              //
/** The boost test suite.
 */
template <typename T>
class DOOCSProcessVariableArrayTestSuite : public test_suite
{

public:

    DOOCSProcessVariableArrayTestSuite() : test_suite("DOOCSProcessVariableArray test suite")
    {
        boost::shared_ptr< DOOCSProcessVariableArrayTest<T> > 	doocsPVArrayTest( new DOOCSProcessVariableArrayTest<T> );
        
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::testSizeEmpty,          doocsPVArrayTest ) );
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::testFill,               doocsPVArrayTest ) );
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::testSetWithoutCallback, doocsPVArrayTest ) );
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::testSet,                doocsPVArrayTest ) );
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::testGetWithoutCallback, doocsPVArrayTest ) );
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::testGet,                doocsPVArrayTest ) );
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::test_fillvector,        doocsPVArrayTest ) );
        add( BOOST_CLASS_TEST_CASE( &DOOCSProcessVariableArrayTest<T>::testAssignment,         doocsPVArrayTest ) );
    }
};
                                                                                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>
                                                                                                                                              //
test_suite * init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
    framework::master_test_suite().p_name.value = "DOOCSProcessVariableArray test suite";
    
    framework::master_test_suite().add( new DOOCSProcessVariableArrayTestSuite<int>   );
    framework::master_test_suite().add( new DOOCSProcessVariableArrayTestSuite<double>);
    framework::master_test_suite().add( new DOOCSProcessVariableArrayTestSuite<float> );
    
    return NULL;
}
                                                                                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>

