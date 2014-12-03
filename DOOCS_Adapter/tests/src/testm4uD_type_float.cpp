#define BOOST_TEST_MODULE test_float




#include "eq_fct.h"

#include "m4uD_type.hpp"




// ============================================================================


struct CallbacksTestFixture {
    
    mtca4u::m4uD_type<float, D_float>                              * mydtype;


    unsigned int _get_cb_counter;
    unsigned int _set_cb_counter;
    unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : _get_cb_counter       (0),
                                     _set_cb_counter       (0),
                                     _set_cb_counter_equals(0)
            {
                mydtype       = new mtca4u::m4uD_type<float, D_float> ( NULL, NULL );
            }
    
            ~CallbacksTestFixture()
            {
                delete mydtype;
            }
    
        

    void    reset_counters()
            {
                _get_cb_counter        = 0;
                _set_cb_counter        = 0;
                _set_cb_counter_equals = 0;
            }
    

            // callbacks
    float   on_get_callback ()                                               //~  < T () >
            {
                ++_get_cb_counter;
                return 0;
            }
    void    on_set_callback (float const & newValue, float const & oldValue) //~  < void (T const &, T const & ) >
            {
                if (newValue == oldValue) ++_set_cb_counter_equals;
                ++_set_cb_counter;
            }
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "m4uD_type_testCases_numericals.hpp"
