#define BOOST_TEST_MODULE test_int




#include "m4uD_type.hpp"

#include "D_int_mock.hpp"




// ============================================================================


struct CallbacksTestFixture {
    
    m4uD_type<int, D_int>                              * mydtype;


    unsigned int _get_cb_counter;
    unsigned int _set_cb_counter;
    unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : _get_cb_counter       (0),
                                     _set_cb_counter       (0),
                                     _set_cb_counter_equals(0)
            {
                mydtype       = new m4uD_type<int, D_int> ( NULL, NULL );
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
