#define BOOST_TEST_MODULE test_array_int

#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_tools.hpp>
using namespace boost::unit_test;



#include <eq_fct.h>
#include <ControlSystemAdapter/StubProcessArray.h>
#include <ControlSystemAdapter/StubProcessVariableFactory.h>       //FIXME - shouldn't be here in the end
#include "m4uD_array.hpp"




// ============================================================================


typedef   std::vector<int>  TypedVector;


struct CallbacksTestFixture {
    
    mtca4u::m4uD_array<int>                  mydarray;


    unsigned int _get_cb_counter;
    unsigned int _set_cb_counter;

    std::vector<int> vec_test;


    mtca4u::StubProcessVariableFactory variableFactory;
    boost::shared_ptr< mtca4u::ProcessArray<int> > pa;     // FIXME


            CallbacksTestFixture() :  mydarray( NULL, 4, NULL )
                                     ,_get_cb_counter       (0)
                                     ,_set_cb_counter       (0)
                                     ,vec_test              (4)
                                     ,pa( variableFactory.getProcessArray<int>("pa",5) )
            {}
    
            ~CallbacksTestFixture() {}
    
        

            // callbacks
    void    on_set_callback (mtca4u::ProcessArray<int> const & ) //~  < void (ProcessArray<T> const & ) >
            {
                ++_set_cb_counter;
                mydarray.fill_spectrum (0, 5.0);
            }
    void    on_get_callback (mtca4u::ProcessArray<int> & )       //~  < void (ProcessArray<T> & ) >
            {
                ++_get_cb_counter;
                mydarray.fill_spectrum (3, 6.0);
            }
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#include "m4uD_array_testCases_numericals.hpp"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


static const int32_t MIN_INT_IN_FLOAT = -16777216;
static const int32_t MAX_INT_IN_FLOAT =  16777215;


BOOST_FIXTURE_TEST_CASE( test_limitedPrecision, CallbacksTestFixture )
{
    for (int32_t i=16777200; i<=16777213; i++)
    {
        mydarray.fill_spectrum( (i%mydarray.length()), i);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), false);
    }
    
    mydarray.fill_spectrum( (16777214 % mydarray.length()), 16777214);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), false);
    mydarray.fill_spectrum( (16777215 % mydarray.length()), 16777215);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), false);
    mydarray.fill_spectrum( (16777216 % mydarray.length()), 16777216);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), true);
    mydarray.fill_spectrum( (16777217 % mydarray.length()), 16777217);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), true);

    for (int32_t i=16777218; i<16777300; i++)
    {
        mydarray.fill_spectrum( (i%mydarray.length()), i);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), true);
    }


    mydarray.fill(0);

    for (int32_t i=16777200; i<=16777213; i++)
    {
        mydarray.fill_spectrum( (i%mydarray.length()), -i);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), false);
    }
    
    mydarray.fill_spectrum( (16777214 % mydarray.length()), -16777214);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), false);
    mydarray.fill_spectrum( (16777215 % mydarray.length()), -16777215);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), false);
    mydarray.fill_spectrum( (16777216 % mydarray.length()), -16777216);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), true);
    mydarray.fill_spectrum( (16777217 % mydarray.length()), -16777217);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), true);

    for (int32_t i=16777218; i<16777300; i++)
    {
        mydarray.fill_spectrum( (i%mydarray.length()), -i);
        BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), true);
    }
}
