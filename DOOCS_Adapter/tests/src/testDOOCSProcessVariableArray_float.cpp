#define BOOST_TEST_MODULE test_dpva_arr


#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

#include "dice.hpp"


#include "eq_fct.h"
#include "ProcessArray.h"
#include "StubProcessArray.h"       //FIXME - shouldn't be here in the end
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



typedef   std::vector<float> const &   CVRef;
typedef   std::vector<float>           Vectorf;


struct TestFixture {
    
    size_t const N_ELEMENTS;
    float  const G_FILL_VALUE; // for gCALLBACK
    
    std::vector<float> referenceVector;
    std::vector<float> toolarge_referenceVector;
    
    unsigned int __setCallbackCounter;
    unsigned int __getCallbackCounter;
    
    boost::shared_ptr< mtca4u::m4uD_array<float> >   darray;            mtca4u::DOOCSProcessVariableArray<float, mtca4u::m4uD_array<float> >  doocs_process_array;
    boost::shared_ptr< mtca4u::m4uD_array<float> >   emptydarray;       mtca4u::DOOCSProcessVariableArray<float, mtca4u::m4uD_array<float> >  empty_doocs_process_array;
    boost::shared_ptr< mtca4u::m4uD_array<float> >   refdarray;         mtca4u::DOOCSProcessVariableArray<float, mtca4u::m4uD_array<float> >  reference_doocs_process_array;
    //~ boost::shared_ptr< mtca4u::m4uD_array<float> >   atrefdarray;                  AssignTestProcessArray<float, mtca4u::m4uD_array<float> >  assign_test_reference_process_array;        // iterators required for this to work
    
    boost::shared_ptr< mtca4u::m4uD_array<float> >   toolarge1darray;   mtca4u::DOOCSProcessVariableArray<float, mtca4u::m4uD_array<float> >  toolarge_ref_doocs_process_array;
    boost::shared_ptr< mtca4u::m4uD_array<float> >   toolarge2darray;              AssignTestProcessArray<float, mtca4u::m4uD_array<float> >  toolarge_assign_test_ref_process_array;
    
    mtca4u::ProcessArray<float> & _process_array;
    mtca4u::ProcessArray<float> & _reference_process_array;


    
    TestFixture() :  N_ELEMENTS               (3)
                    ,G_FILL_VALUE             (50)
                    //~ ,SOME_NUMBER(42)
                    
                    ,referenceVector          (N_ELEMENTS, 6)
                    ,toolarge_referenceVector (N_ELEMENTS+1, 6)
                    
                    ,__setCallbackCounter     (0)
                    ,__getCallbackCounter     (0)
                    
                    ,darray          ( new mtca4u::m4uD_array<float> ( NULL, N_ELEMENTS, NULL ) )      ,doocs_process_array                    (darray          , N_ELEMENTS)
                    ,emptydarray     ( new mtca4u::m4uD_array<float> ( NULL,          0, NULL ) )      ,empty_doocs_process_array              (emptydarray     ,          0)
                    ,refdarray       ( new mtca4u::m4uD_array<float> ( NULL, N_ELEMENTS, NULL ) )      ,reference_doocs_process_array          (refdarray       , N_ELEMENTS)
                    //~ ,atrefdarray     ( new mtca4u::m4uD_array<float> ( NULL, N_ELEMENTS, NULL ) )      ,assign_test_reference_process_array    (atrefdarray     , N_ELEMENTS)        // iterators required for this to work
                    
                    ,toolarge1darray ( new mtca4u::m4uD_array<float> ( NULL, N_ELEMENTS+1, NULL ) )    ,toolarge_ref_doocs_process_array       (toolarge1darray , N_ELEMENTS+1)
                    ,toolarge2darray ( new mtca4u::m4uD_array<float> ( NULL, N_ELEMENTS+1, NULL ) )    ,toolarge_assign_test_ref_process_array (toolarge2darray , N_ELEMENTS+1)
                    
                    ,_process_array(doocs_process_array)
                    ,_reference_process_array(reference_doocs_process_array)
    {
        _process_array.setOnSetCallbackFunction( boost::bind( &TestFixture::setCBfun, this, _1) );
        _process_array.setOnGetCallbackFunction( boost::bind( &TestFixture::getCBfun, this, _1) );
    }
    
    ~TestFixture()
    {
    }
    
    
    // sCALLBACK
    void setCBfun(mtca4u::ProcessArray<float> const & ){
        ++__setCallbackCounter;
    }
    
    // gCALLBACK
    void getCBfun(mtca4u::ProcessArray<float> & toBeFilled ){
        toBeFilled.fill(G_FILL_VALUE);
        ++__getCallbackCounter;  
    }

                                                                                    // FIXME: add CS-side activity mock (D_der'v level)
};





BOOST_FIXTURE_TEST_CASE( testConstructors, TestFixture )
{
    BOOST_CHECK_EQUAL ( doocs_process_array.size() , N_ELEMENTS );
    BOOST_CHECK_EQUAL ( doocs_process_array.empty(), false      );

    BOOST_CHECK_EQUAL ( empty_doocs_process_array.size() , 0    );
    BOOST_CHECK_EQUAL ( empty_doocs_process_array.empty(), true );


    BOOST_CHECK_EQUAL ( darray->length()      , N_ELEMENTS );
    BOOST_CHECK_EQUAL ( emptydarray->length() , 0          );
}



/*BOOST_FIXTURE_TEST_CASE( testIterators, TestFixture )
{
    int i;

    mtca4u::DOOCSProcessVariableFactory DOOCSPVFactory( NULL );
    
    boost::shared_ptr<mtca4u::ProcessVariable<int> > integer1 = DOOCSPVFactory.getProcessVariable<int> ("Integer1");
    boost::shared_ptr<mtca4u::ProcessVariable<int> > integer2 = DOOCSPVFactory.getProcessVariable<int> ("Integer2");
    boost::shared_ptr<mtca4u::ProcessVariable<int> > integer3 = DOOCSPVFactory.getProcessVariable<int> ("Integer3");

    std::vector<int> testValsContainer;
    testValsContainer.push_back(1);
    testValsContainer.push_back(2);
    testValsContainer.push_back(3);

    integer1->setWithoutCallback(testValsContainer[0]);
    integer2->setWithoutCallback(testValsContainer[1]);
    integer3->setWithoutCallback(testValsContainer[2]);


    // use the fact that we're friend and can directly access the underlying vector
    _testedArrayContainer[0] = integer1;
    _testedArrayContainer[1] = integer2;
    _testedArrayContainer[2] = integer3;
    //~ for (size_t j = 0; j < N_ELEMENTS; ++j) {
    //~ }

    int tmp_idx = 0;
    for (typename mtca4u::ProcessArray<int>::iterator it= _processArray.begin(); it != _processArray.end(); ++it) {
        //~ *it=i++;
        BOOST_CHECK_EQUAL( *it, static_cast<int>(j+SOME_NUMBER) );
    }


    //constant array with begin and end
    i = SOME_NUMBER;
    for (typename mtca4u::ProcessArray<int>::const_iterator it= _constArray.begin(); it != _constArray.end(); ++it){
        BOOST_CHECK_EQUAL( *it, i++ );    
    }

    //non-constant array with const_iterator
    i = SOME_NUMBER;
    for (typename mtca4u::ProcessArray<int>::const_iterator it= _processArray.cbegin(); it != _processArray.cend(); ++it){
        BOOST_CHECK_EQUAL( *it, i++ );    
    }

    // check that the iterators run with std algorithms
    std::sort( _processArray.rbegin(), _processArray.rend() );

    // again use the container to check on the modifications
    i = SOME_NUMBER+N_ELEMENTS;
    for (size_t j = 0; j < _testedArrayContainer.size(); ++j) {
        BOOST_CHECK_EQUAL( _testedArrayContainer[j], static_cast<int>(--i) );
    }

    //constant array with rbegin and rend
    i = SOME_NUMBER;
    for (typename mtca4u::ProcessArray<int>::const_reverse_iterator it= _constArray.rbegin(); it != _constArray.rend(); ++it) {
        BOOST_CHECK_EQUAL( *it, i++ );    
    }

    //non-constant array with const_iterator
    i = SOME_NUMBER;
    for (typename mtca4u::ProcessArray<int>::const_reverse_iterator it= _processArray.crbegin(); it != _processArray.crend(); ++it) {
        BOOST_CHECK_EQUAL( *it, i++ );    
    }
}
*/


// FIXME: unable to obtain direct reference to the array element
/*BOOST_FIXTURE_TEST_CASE( testRandomAccess, TestFixture )
{
    int i;
    
    i = SOME_NUMBER;
    // set via [] operator
    for (size_t j = 0; j < _processArray.size(); ++j) {
        _processArray[j] = static_cast<T>(i++);
    }
    //~ // FIXME: no [] operator
    //~ 
    // check the container for verification
    i = SOME_NUMBER;
    for (size_t j = 0; j < _stubProcessArray._container.size(); ++j){
    BOOST_CHECK( _stubProcessArray._container[j] = static_cast<T>(i++) );
    }
    //~ 
    i = SOME_NUMBER;
    // check via const [] operator
    for (size_t j = 0; j < _processArray.size(); ++j){
    BOOST_CHECK( _constArray[j] == static_cast<T>(i++) );
    }  
    //~ // FIXME: no const [] operator
//~ 
    //~ 
    // now the same with at()
    // setting:
        darray->fill_spectrum (0, 4.0);
        darray->fill_spectrum (1, 5.0);
        darray->fill_spectrum (2, 6.0);
        darray->fill_spectrum (3, 7.0);

    i = SOME_NUMBER + _processArray.size();
    for (size_t j = 0; j < _processArray.size(); ++j){
    _processArray.at(j) = static_cast<T>(--i);
    }
    
    // check the container for verification
    i = SOME_NUMBER + _processArray.size();
    for (size_t j = 0; j < _stubProcessArray._container.size(); ++j){
    BOOST_CHECK( _stubProcessArray._container[j] = static_cast<T>(--i) );
    }
    
    i = SOME_NUMBER + _processArray.size();
    // check via const [] operator
    for (size_t j = 0; j < _processArray.size(); ++j){
    BOOST_CHECK( _constArray[j] == static_cast<T>(--i) );
    }  
    
    // check the out of range check
    BOOST_CHECK_THROW( _processArray.at(N_ELEMENTS), std::out_of_range);
    BOOST_CHECK_THROW( _constArray.at(N_ELEMENTS), std::out_of_range);
}*/



// FIXME: unable to obtain direct reference to the array element
/*BOOST_FIXTURE_TEST_CASE( testFrontBack, TestFixture )
{
    darray->fill_spectrum (0, 4.0);
    darray->fill_spectrum (1, 5.0);
    darray->fill_spectrum (2, 6.0);
    darray->fill_spectrum (3, 7.0);

  BOOST_CHECK( _constArray.front() == static_cast<T>(SOME_NUMBER + N_ELEMENTS -1) );
  ++_processArray.front();
  // use the container to verify
  BOOST_CHECK( _stubProcessArray._container.front() == static_cast<T>(SOME_NUMBER + N_ELEMENTS) );

  BOOST_CHECK( _constArray.back() == static_cast<T>(SOME_NUMBER) );
  --_processArray.back();
  // use the container to verify
  BOOST_CHECK( _stubProcessArray._container.back() == static_cast<T>(SOME_NUMBER -1) );
}*/


BOOST_FIXTURE_TEST_CASE( testFill, TestFixture )
{
    doocs_process_array.fill(30);
    
    for(int i=0; i<darray->length(); ++i)
        BOOST_CHECK_EQUAL(darray->read_spectrum (i), 30);
}


BOOST_FIXTURE_TEST_CASE( testSetWithoutCallback, TestFixture )
{
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


BOOST_FIXTURE_TEST_CASE( testSet, TestFixture )
{
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



BOOST_FIXTURE_TEST_CASE( testGetWithoutCallback, TestFixture )
{
    // fill something known so we can check that it changed
    _process_array.fill(5);


    CVRef result_of_get = doocs_process_array.getWithoutCallback();

    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL( result_of_get[i], darray->read_spectrum ((int)i));
    }


    //~ _process_array.fill(4);


    //~ result_of_get = _process_array.getWithoutCallback();        FIXME: class mtca4u::ProcessArray<float>’ has no member named ‘getWithoutCallback' - ProcessArray interface change required
//~ 
    //~ for (size_t i=0; i<N_ELEMENTS; ++i)
    //~ {
        //~ BOOST_CHECK_EQUAL( result_of_get[i], darray->read_spectrum ((int)i));
    //~ }


    // FIXME: assign_test_reference_process_array?

    BOOST_CHECK_EQUAL( __setCallbackCounter, 0 );
    BOOST_CHECK_EQUAL( __getCallbackCounter, 0 );
}


BOOST_FIXTURE_TEST_CASE( testGet, TestFixture )
{
    // fill something known so we can check that it changed
    _process_array.fill(5);


    CVRef result_of_get1 = doocs_process_array.get();
    BOOST_CHECK_EQUAL( __getCallbackCounter, 1 );
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL( G_FILL_VALUE, darray->read_spectrum ((int)i) );
        BOOST_CHECK_EQUAL( G_FILL_VALUE, result_of_get1[i] );
    }

    
    // clear the getter callback function
    _process_array.clearOnGetCallbackFunction();
    _process_array.fill(5);


    CVRef result_of_get2 = doocs_process_array.get();
    BOOST_CHECK_EQUAL( __getCallbackCounter, 1 );
    for (size_t i=0; i<N_ELEMENTS; ++i)
    {
        BOOST_CHECK_EQUAL( 5, darray->read_spectrum ((int)i) );
        BOOST_CHECK_EQUAL( 5, result_of_get2[i] );
    }
}


BOOST_FIXTURE_TEST_CASE( test_fillvector, TestFixture )
{
    _process_array = referenceVector;


    Vectorf v(N_ELEMENTS);
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


BOOST_FIXTURE_TEST_CASE( testAssignment, TestFixture )
{
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
