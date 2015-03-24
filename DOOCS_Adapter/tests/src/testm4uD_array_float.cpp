#define BOOST_TEST_MODULE test_array_float

#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_tools.hpp>
using namespace boost::unit_test;



#include "eq_fct.h"
#include "ProcessArray.h"
#include "StubProcessArray.h"       //FIXME - shouldn't be here in the end
#include "m4uD_array.hpp"




// ============================================================================


typedef   std::vector<float>  TypedVector;


struct CallbacksTestFixture {
    
    mtca4u::m4uD_array<float>                  mydarray;


    unsigned int _get_cb_counter;
    unsigned int _set_cb_counter;

    std::vector<float> vec_test;
    
    
    mtca4u::StubProcessArray<float> pa;     // FIXME


            CallbacksTestFixture() :  mydarray( NULL, 4, NULL )
                                     ,_get_cb_counter       (0)
                                     ,_set_cb_counter       (0)
                                     ,vec_test              (4)
                                     ,pa(5)
            {}
    
            ~CallbacksTestFixture() {}
    
        

            // callbacks
    void    on_set_callback (mtca4u::ProcessArray<float> const & ) //~  < void (ProcessArray<T> const & ) >
            {
                ++_set_cb_counter;
                mydarray.fill_spectrum (0, 5.0);
            }
    void    on_get_callback (mtca4u::ProcessArray<float> & )       //~  < void (ProcessArray<T> & ) >
            {
                ++_get_cb_counter;
                mydarray.fill_spectrum (3, 6.0);
            }
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#include "m4uD_array_testCases_numericals.hpp"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


BOOST_FIXTURE_TEST_CASE( test_limitedPrecision, CallbacksTestFixture )
{
    BOOST_CHECK_EQUAL(mydarray.limitedPrecision(), false);
}
