#define BOOST_TEST_MODULE test_int




#include "eq_fct.h"

#include "m4uD_type.hpp"




// ============================================================================


struct CallbacksTestFixture {
    
    mtca4u::m4uD_type<int, D_int>   mydtype;


    unsigned int _get_cb_counter;
    unsigned int _set_cb_counter;
    unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : mydtype( NULL, NULL ),
                                     _get_cb_counter       (0),
                                     _set_cb_counter       (0),
                                     _set_cb_counter_equals(0)
            {}
    
            ~CallbacksTestFixture() {}
    
        

    void    reset_counters()
            {
                _get_cb_counter        = 0;
                _set_cb_counter        = 0;
                _set_cb_counter_equals = 0;
            }
    

            // callbacks
    int     on_get_callback ()                                           //~  < T () >
            {
                ++_get_cb_counter;
                return 0;
            }
    void    on_set_callback (int const & newValue, int const & oldValue) //~  < void (T const &, T const & ) >
            {
                if (newValue == oldValue) ++_set_cb_counter_equals;
                ++_set_cb_counter;
            }
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "m4uD_type_testCases_numericals.hpp"
