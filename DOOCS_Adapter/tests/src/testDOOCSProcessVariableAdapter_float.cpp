#define BOOST_TEST_MODULE callback_model_test


#include "D_float_mock.hpp"
#include "D_string_mock.hpp"

#include "m4uD_type.hpp"
#include "DOOCSProcessVariableAdapter.hpp"




struct CallbacksTestFixture {
	
    mtca4u::DOOCSPVAdapter<float, m4uD_type<float, D_float> > * doocs_adapter;
    m4uD_type<float, D_float>                                 * mydtype;


	unsigned int _get_cb_counter;
	unsigned int _set_cb_counter;
	unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : _get_cb_counter       (0),
                                     _set_cb_counter       (0),
                                     _set_cb_counter_equals(0)
            {
                mydtype       = new m4uD_type<float, D_float> ( NULL, NULL );
                doocs_adapter = new mtca4u::DOOCSPVAdapter<float, m4uD_type<float, D_float> > (mydtype);
            }
    
            ~CallbacksTestFixture()
            {
                delete doocs_adapter;
                delete mydtype;
            }
    
    	

	void	reset_counters()
			{
				_get_cb_counter        = 0;
				_set_cb_counter        = 0;
				_set_cb_counter_equals = 0;
			}
	

            // callbacks
	float	on_get_callback ()	                                             //~  < T () >
			{
				++_get_cb_counter;
				return 0;
			}
	void	on_set_callback (float const & newValue, float const & oldValue) //~  < void (T const &, T const & ) >
			{
				if (newValue == oldValue) ++_set_cb_counter_equals;
				++_set_cb_counter;
			}
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


struct InterPVTestFixture {         // for testing interactions between two PVs,
                                    // like setting or assigning from a PV
    mtca4u::DOOCSPVAdapter<float, m4uD_type<float, D_float> > * doocs_adapter1;
    mtca4u::DOOCSPVAdapter<float, m4uD_type<float, D_float> > * doocs_adapter2;
    
    m4uD_type<float, D_float>                                 * mydtype1;
    m4uD_type<float, D_float>                                 * mydtype2;


	unsigned int _get_cb_counter1;
	unsigned int _set_cb_counter1;
	unsigned int _set_cb_counter_equals1;
	unsigned int _get_cb_counter2;
	unsigned int _set_cb_counter2;
	unsigned int _set_cb_counter_equals2;



            InterPVTestFixture() : _get_cb_counter1       (0),
                                   _set_cb_counter1       (0),
                                   _set_cb_counter_equals1(0),
                                   _get_cb_counter2       (0),
                                   _set_cb_counter2       (0),
                                   _set_cb_counter_equals2(0)
            {
                mydtype1       = new m4uD_type<float, D_float> ( NULL, NULL );
                mydtype2       = new m4uD_type<float, D_float> ( NULL, NULL );
                doocs_adapter1 = new mtca4u::DOOCSPVAdapter<float, m4uD_type<float, D_float> > (mydtype1);
                doocs_adapter2 = new mtca4u::DOOCSPVAdapter<float, m4uD_type<float, D_float> > (mydtype2);
            }
    
            ~InterPVTestFixture()
            {
                delete doocs_adapter1;
                delete doocs_adapter2;
                delete mydtype1;
                delete mydtype2;
            }
    
    	

	void	reset_counters()
			{
				_get_cb_counter1        = 0;
				_set_cb_counter1        = 0;
				_set_cb_counter_equals1 = 0;
				_get_cb_counter2        = 0;
				_set_cb_counter2        = 0;
				_set_cb_counter_equals2 = 0;
			}
	

            // callbacks
            // set 1
	float   on_get_callback1 ()	                                              //~  < T () >
			{
				++_get_cb_counter1;
				return 0;
			}
	void	on_set_callback1 (float const & newValue, float const & oldValue) //~  < void (T const &, T const & ) >
			{
				if (newValue == oldValue) ++_set_cb_counter_equals1;
				++_set_cb_counter1;
			}
            // set 2 (NOTE: different treating of counters!)
	float	on_get_callback2 ()	                                              //~  < T () >
			{
				_get_cb_counter2 += 2;
				return 0;
			}
	void	on_set_callback2 (float const & newValue, float const & oldValue) //~  < void (T const &, T const & ) >
			{
				if (newValue == oldValue) _set_cb_counter_equals2 += 2;
				_set_cb_counter2 += 2;
			}
};


// ============================================================================
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ============================================================================


#include "DOOCSProcessVariableAdapter_testCases_numericals.hpp"
