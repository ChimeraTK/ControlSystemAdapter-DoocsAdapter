#define BOOST_TEST_MODULE test_dpva_f


#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

#include "DOOCSProcessVariableFactory.hpp"


#include <vector>



static const size_t ARRAY_SIZE       = 15;
static const size_t WRONG_ARRAY_SIZE = 35;



BOOST_AUTO_TEST_CASE( testGetProcessVariable_basicoperation )
{
    mtca4u::DOOCSProcessVariableFactory DOOCSPVFactory( NULL );
    
    boost::shared_ptr<mtca4u::ProcessVariable<int> >         Integer = DOOCSPVFactory.getProcessVariable<int>        ("Integer");
    boost::shared_ptr<mtca4u::ProcessVariable<double> >      Double  = DOOCSPVFactory.getProcessVariable<double>     ("Double");
    boost::shared_ptr<mtca4u::ProcessVariable<float> >       Float   = DOOCSPVFactory.getProcessVariable<float>      ("Float");
    boost::shared_ptr<mtca4u::ProcessVariable<std::string> > String  = DOOCSPVFactory.getProcessVariable<std::string>("String");

    Integer->set(1);                       BOOST_CHECK_EQUAL(Integer->get(), 1);
    Double ->set(2);                       BOOST_CHECK_EQUAL(Double->get() , 2);
    Float  ->set(3);                       BOOST_CHECK_EQUAL(Float->get()  , 3);
    String ->set(std::string("teststr"));  BOOST_CHECK_EQUAL(String->get() , std::string("teststr"));
}


BOOST_AUTO_TEST_CASE( testGetProcessVariable_map )
{
    mtca4u::DOOCSProcessVariableFactory DOOCSPVFactory( NULL );

    boost::shared_ptr<mtca4u::ProcessVariable<int> > firstInteger  = DOOCSPVFactory.getProcessVariable<int>("firstInteger");  // 1
    boost::shared_ptr<mtca4u::ProcessVariable<int> > secondInteger = DOOCSPVFactory.getProcessVariable<int>("firstInteger");  // 1
    boost::shared_ptr<mtca4u::ProcessVariable<int> > thirdInteger  = DOOCSPVFactory.getProcessVariable<int>("thirdInteger");  // 2

    firstInteger->set(42);
    thirdInteger->set(55);

    // make sure the second integer is the same instance as the first integer, but the third is not
    BOOST_CHECK_EQUAL(*secondInteger, 42);
    secondInteger->set(43);
    BOOST_CHECK_EQUAL(*firstInteger, 43);
    BOOST_CHECK_EQUAL(*thirdInteger, 55);

    // exceptions
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessVariable<uint64_t>("tooLong"), std::bad_typeid );
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessVariable<double>("firstInteger"), std::bad_cast );  // not boost::bad_any_cast
    
    // check that name conflicts with arrays work
    boost::shared_ptr<mtca4u::ProcessArray<int> > intArray = DOOCSPVFactory.getProcessArray<int>("intArray", ARRAY_SIZE);
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessVariable<int>("intArray"),  boost::bad_any_cast );
}



BOOST_AUTO_TEST_CASE( testGetProcessArray_basicoperation )
{
    mtca4u::DOOCSProcessVariableFactory DOOCSPVFactory( NULL );
    
    boost::shared_ptr<mtca4u::ProcessArray<int> >         IntegerA = DOOCSPVFactory.getProcessArray<int>        ("Integer", ARRAY_SIZE);
    boost::shared_ptr<mtca4u::ProcessArray<double> >      DoubleA  = DOOCSPVFactory.getProcessArray<double>     ("Double" , ARRAY_SIZE);
    boost::shared_ptr<mtca4u::ProcessArray<float> >       FloatA   = DOOCSPVFactory.getProcessArray<float>      ("Float"  , ARRAY_SIZE);

    IntegerA->fill(1);
    DoubleA ->fill(2);
    FloatA  ->fill(3);

    //~ the below sequence is really ugly. The aim is to read the arrays contents. So:
    //~ 1. getting the raw pointer
    mtca4u::ProcessArray<int>    const * IntegerA_raw = IntegerA.get();
    mtca4u::ProcessArray<double> const * DoubleA_raw  = DoubleA.get();
    mtca4u::ProcessArray<float>  const * FloatA_raw   = FloatA.get();
    //~ 2. typecast to DOOCSProcessVariableArray const *
    mtca4u::DOOCSProcessVariableArray< int   , mtca4u::m4uD_array<int   > > const * DPVA_IntegerA_rawconst = static_cast< mtca4u::DOOCSProcessVariableArray< int   , mtca4u::m4uD_array<int> >    const * >( IntegerA_raw );
    mtca4u::DOOCSProcessVariableArray< double, mtca4u::m4uD_array<double> > const * DPVA_DoubleA_rawconst  = static_cast< mtca4u::DOOCSProcessVariableArray< double, mtca4u::m4uD_array<double> > const * >( DoubleA_raw );
    mtca4u::DOOCSProcessVariableArray< float , mtca4u::m4uD_array<float > > const * DPVA_FloatA_rawconst   = static_cast< mtca4u::DOOCSProcessVariableArray< float , mtca4u::m4uD_array<float> >  const * >( FloatA_raw );
    //~ 3. de-const'ing to DOOCSProcessVariableArray *
    mtca4u::DOOCSProcessVariableArray< int   , mtca4u::m4uD_array<int> >          * DPVA_IntegerA_raw      =  const_cast< mtca4u::DOOCSProcessVariableArray< int   , mtca4u::m4uD_array<int> >          * >( DPVA_IntegerA_rawconst );
    mtca4u::DOOCSProcessVariableArray< double, mtca4u::m4uD_array<double> >       * DPVA_DoubleA_raw       =  const_cast< mtca4u::DOOCSProcessVariableArray< double, mtca4u::m4uD_array<double> >       * >( DPVA_DoubleA_rawconst );
    mtca4u::DOOCSProcessVariableArray< float , mtca4u::m4uD_array<float> >        * DPVA_FloatA_raw        =  const_cast< mtca4u::DOOCSProcessVariableArray< float , mtca4u::m4uD_array<float> >        * >( DPVA_FloatA_rawconst );
    //~ only now get() is accessible
    const std::vector<int>    & intV    = DPVA_IntegerA_raw->get();
    const std::vector<double> & doubleV = DPVA_DoubleA_raw ->get();
    const std::vector<float>  & floatV  = DPVA_FloatA_raw  ->get();
    //~ the reason for such way is that the current ProcessArray interface has no get()'s,
    //~ while at the same time the nature of DOOCS data storage prevents iterators implementation
    //~ Off course this should not be like that in the end!
    
    for (std::vector<int>   ::const_iterator it = intV   .begin(); it != intV   .end(); ++it)   BOOST_CHECK(*it == 1);
    for (std::vector<double>::const_iterator it = doubleV.begin(); it != doubleV.end(); ++it)   BOOST_CHECK(*it == 2);
    for (std::vector<float> ::const_iterator it = floatV .begin(); it != floatV .end(); ++it)   BOOST_CHECK(*it == 3);
}


BOOST_AUTO_TEST_CASE( testGetProcessArray_map )
{
    mtca4u::DOOCSProcessVariableFactory DOOCSPVFactory( NULL );

    boost::shared_ptr<mtca4u::ProcessArray<int> > firstIntegerA  = DOOCSPVFactory.getProcessArray<int>("firstInteger", ARRAY_SIZE);  // 1
    boost::shared_ptr<mtca4u::ProcessArray<int> > secondIntegerA = DOOCSPVFactory.getProcessArray<int>("firstInteger", ARRAY_SIZE);  // 1
    boost::shared_ptr<mtca4u::ProcessArray<int> > thirdIntegerA  = DOOCSPVFactory.getProcessArray<int>("thirdInteger", ARRAY_SIZE);  // 2

    firstIntegerA->fill(42);
    thirdIntegerA->fill(55);


    mtca4u::ProcessArray<int> const * firstIntegerA_raw  = firstIntegerA.get();
    mtca4u::ProcessArray<int> const * secondIntegerA_raw = secondIntegerA.get();
    mtca4u::ProcessArray<int> const * thirdIntegerA_raw  = thirdIntegerA.get();

    mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * DPVA_firstIntegerA_rawconst  = static_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * >( firstIntegerA_raw );
    mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * DPVA_secondIntegerA_rawconst = static_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * >( secondIntegerA_raw );
    mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * DPVA_thirdIntegerA_rawconst  = static_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * >( thirdIntegerA_raw );

    mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> >       * DPVA_firstIntegerA_raw       =  const_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > * >( DPVA_firstIntegerA_rawconst );
    mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> >       * DPVA_secondIntegerA_raw      =  const_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > * >( DPVA_secondIntegerA_rawconst );
    mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> >       * DPVA_thirdIntegerA_raw       =  const_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > * >( DPVA_thirdIntegerA_rawconst );

    const std::vector<int> & firstV  = DPVA_firstIntegerA_raw ->get();
    const std::vector<int> & secondV = DPVA_secondIntegerA_raw->get();
    const std::vector<int> & thirdV  = DPVA_thirdIntegerA_raw ->get();


    // make sure the second integer array is the same instance as the first one, but the third is not
    for (std::vector<int>::const_iterator it = secondV.begin(); it != secondV.end(); ++it)   BOOST_CHECK(*it == 42);
    secondIntegerA->fill(43);
    DPVA_firstIntegerA_raw ->get();     // sync cache
    for (std::vector<int>::const_iterator it = firstV.begin(); it != firstV.end(); ++it)   BOOST_CHECK(*it == 43);
    DPVA_thirdIntegerA_raw ->get();     // sync cache
    for (std::vector<int>::const_iterator it = thirdV.begin(); it != thirdV.end(); ++it)   BOOST_CHECK(*it == 55);

    // exceptions
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessArray<uint64_t>("unknownType" , ARRAY_SIZE)      , std::bad_typeid ); // unknown type
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessArray<double>  ("firstInteger", ARRAY_SIZE)      , std::bad_cast );  // not boost::bad_any_cast
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessArray<int>     ("firstInteger", WRONG_ARRAY_SIZE), std::length_error );

    // check that name conflicts with variables work
    boost::shared_ptr<mtca4u::ProcessVariable<int> > firstInt = DOOCSPVFactory.getProcessVariable<int>("firstInt");
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessArray<int>("firstInt", ARRAY_SIZE), boost::bad_any_cast );
}

