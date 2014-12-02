#define BOOST_TEST_MODULE callback_model_test
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "D_float_mock.hpp"

#include "m4uD_type.hpp"
#include "DOOCSProcessVariableAdapter.hpp"





// ============================================================================


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


BOOST_FIXTURE_TEST_SUITE( test_callbacks, CallbacksTestFixture )

BOOST_AUTO_TEST_CASE( test_get_cb_count )
{
    reset_counters();

    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 1 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    doocs_adapter->clearOnGetCallbackFunction();
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 2 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 3 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 4 );


    mydtype->clearOnGetCallbackFunction();
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 4 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 4 );
}


BOOST_AUTO_TEST_CASE( test_set_cb_count )
{
    reset_counters();

    
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 1 );
    BOOST_CHECK( _set_cb_counter_equals == 1 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );


    doocs_adapter->clearOnSetCallbackFunction();
    
    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );


    mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->set(2);
    BOOST_CHECK( _set_cb_counter        == 3 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );


    mydtype->clearOnSetCallbackFunction();
    
    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );
}

BOOST_AUTO_TEST_CASE( test_sync1 )
{
    reset_counters();

    
    BOOST_CHECK( doocs_adapter->get  () == 0 );
    BOOST_CHECK( mydtype      ->value() == 0 );


    doocs_adapter->set(1);
    BOOST_CHECK( doocs_adapter->get  () == 1 );
    BOOST_CHECK( mydtype      ->value() == 1 );  // <---

    doocs_adapter->set(0);
    BOOST_CHECK( doocs_adapter->get  () == 0 );
    BOOST_CHECK( mydtype      ->value() == 0 );  // <---

    mydtype->set_value(3);
    BOOST_CHECK( mydtype      ->value() == 3 );
    BOOST_CHECK( doocs_adapter->get  () == 3 );  // <---

    mydtype->set_value(0);
    BOOST_CHECK( mydtype      ->value() == 0 );
    BOOST_CHECK( doocs_adapter->get  () == 0 );  // <---
	

	doocs_adapter->set(0);
}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_no_callbacks, CallbacksTestFixture )

BOOST_AUTO_TEST_CASE( test_get_nocb_count )
{
    reset_counters();

    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    doocs_adapter->clearOnGetCallbackFunction();
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydtype->clearOnGetCallbackFunction();
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );
}


BOOST_AUTO_TEST_CASE( test_set_nocb_count )
{
    reset_counters();

    
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    doocs_adapter->clearOnSetCallbackFunction();
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    mydtype->clearOnSetCallbackFunction();
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


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


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


BOOST_FIXTURE_TEST_SUITE( set_from_other_pv, InterPVTestFixture )

BOOST_AUTO_TEST_CASE( set_from_other_pv__set_checking )
{
    // -- given -- 
    reset_counters();
    doocs_adapter1->setWithoutCallback(5);
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 0 );
    
    // -- when --
    doocs_adapter2->set(*doocs_adapter1);
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
}


BOOST_AUTO_TEST_CASE( set_from_other_pv__callbacks_operation )
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    // -- when --
    doocs_adapter2->set(*doocs_adapter1);
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
    
    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 2 );

}

BOOST_AUTO_TEST_CASE( set_from_other_pv__callbacks_assignment ) // or rather for no assignment
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    doocs_adapter1->set(1);                         // do some random get/set() ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 1 );    // ... and check the callback operation so far
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 2 );
    
    // -- when --
    doocs_adapter2->set(*doocs_adapter1);
    
    // -- then --
    
    BOOST_CHECK( _get_cb_counter1        == 1 );    // counters state after set(pv)
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 4 );    // <---

    doocs_adapter1->set(1);                         // do some random get/set() again ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 2 );    // ... and make sure the callback assignment remains unaltered
    BOOST_CHECK( _set_cb_counter1        == 2 );
    BOOST_CHECK( _get_cb_counter2        == 4 );
    BOOST_CHECK( _set_cb_counter2        == 6 );
}

BOOST_AUTO_TEST_SUITE_END()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOST_FIXTURE_TEST_SUITE( setWithoutCallback_from_other_pv, InterPVTestFixture )

BOOST_AUTO_TEST_CASE( setWithoutCallback_from_other_pv__set_checking )
{
    // -- given -- 
    reset_counters();
    doocs_adapter1->setWithoutCallback(5);
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 0 );
    
    // -- when --
    doocs_adapter2->setWithoutCallback(*doocs_adapter1);
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
}


BOOST_AUTO_TEST_CASE( setWithoutCallback_from_other_pv__callbacks_operation )
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    // -- when --
    doocs_adapter2->setWithoutCallback(*doocs_adapter1);
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
    
    BOOST_CHECK( _get_cb_counter1        == 0 );    // make sure callbacks not called
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
}


BOOST_AUTO_TEST_CASE( setWithoutCallback_from_other_pv__callbacks_assignment ) // or rather for no assignment
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    doocs_adapter1->set(1);                         // do some random get/set() ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 1 );    // ... and check the callback operation so far
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 2 );
    
    // -- when --
    doocs_adapter2->setWithoutCallback(*doocs_adapter1);
    
    // -- then --
    BOOST_CHECK( _get_cb_counter1        == 1 );    // counters state after set(pv)
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 2 );    // <---

    doocs_adapter1->set(1);                         // do some random get/set() again ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 2 );    // ... and make sure the callback assignment remains unaltered
    BOOST_CHECK( _set_cb_counter1        == 2 );
    BOOST_CHECK( _get_cb_counter2        == 4 );
    BOOST_CHECK( _set_cb_counter2        == 4 );
}

BOOST_AUTO_TEST_SUITE_END()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOST_FIXTURE_TEST_SUITE( assign_from_other_pv, InterPVTestFixture )

BOOST_AUTO_TEST_CASE( assign_from_other_pv__checking_operation )
{
    // -- given -- 
    reset_counters();
    doocs_adapter1->setWithoutCallback(5);
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 0 );
    
    // -- when --
    *doocs_adapter2 = *doocs_adapter1;
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
}


BOOST_AUTO_TEST_CASE( assign_from_other_pv__callbacks_operation )
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    // -- when --
    *doocs_adapter2 = *doocs_adapter1;
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
    
    BOOST_CHECK( _get_cb_counter1        == 0 );    // make sure callbacks not called
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
}


BOOST_AUTO_TEST_CASE( assign_from_other_pv__callbacks_assignment ) // or rather for no assignment
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    doocs_adapter1->set(1);                         // do some random get/set() ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 1 );    // ... and check the callback operation so far
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 2 );
    
    // -- when --
    *doocs_adapter2 = *doocs_adapter1;
    
    // -- then --
    BOOST_CHECK( _get_cb_counter1        == 1 );    // counters state after set(pv)
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 2 );    // <---

    doocs_adapter1->set(1);                         // do some random get/set() again ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 2 );    // ... and make sure the callback assignment remains unaltered
    BOOST_CHECK( _set_cb_counter1        == 2 );
    BOOST_CHECK( _get_cb_counter2        == 4 );
    BOOST_CHECK( _set_cb_counter2        == 4 );
}

BOOST_AUTO_TEST_SUITE_END()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOST_FIXTURE_TEST_SUITE( assign_from_primtype, InterPVTestFixture )

BOOST_AUTO_TEST_CASE( assign_from_primtype__checking_operation )
{
    // -- given -- 
    reset_counters();
    doocs_adapter2->setWithoutCallback(0);

    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 0 );
    
    // -- when --
    *doocs_adapter2 = 5;
    
    // -- then --
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
}


BOOST_AUTO_TEST_CASE( assign_from_primtype__callbacks_operation )
{
    // -- given -- 
    reset_counters();

    doocs_adapter2->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter2->setWithoutCallback(0);          // some init val

    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2)); // prime callbacks
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    // -- when --
    *doocs_adapter2 = 5;
    
    // -- then --
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
    
    BOOST_CHECK( _get_cb_counter2        == 0 );    // make sure callbacks not called
    BOOST_CHECK( _set_cb_counter2        == 0 );
}


BOOST_AUTO_TEST_CASE( assign_from_primtype__callbacks_assignment ) // or rather for no assignment
{
    // -- given -- 
    reset_counters();

    doocs_adapter2->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter2->setWithoutCallback(0);          // some init vals

    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2)); // prime callbacks
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    doocs_adapter2->set(1);                         // do some random get/set() ...
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter2        == 2 );    // ... and check the callback operation so far
    BOOST_CHECK( _set_cb_counter2        == 2 );
    
    // -- when --
    *doocs_adapter2 = 5;
    
    // -- then --
    BOOST_CHECK( _get_cb_counter2        == 2 );    // counters state after set()
    BOOST_CHECK( _set_cb_counter2        == 2 );    // <---

    doocs_adapter2->set(1);                         // do some random get/set() again ...
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter2        == 4 );    // ... and make sure the callback assignment remains unaltered
    BOOST_CHECK( _set_cb_counter2        == 4 );
}

BOOST_AUTO_TEST_SUITE_END()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOST_FIXTURE_TEST_SUITE( conversion, InterPVTestFixture )

BOOST_AUTO_TEST_CASE( conversion__checking_operation )
{
    // -- given -- 
    reset_counters();
    doocs_adapter1->setWithoutCallback(5);
    doocs_adapter2->setWithoutCallback(4);

    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 4 );
    
    // -- when --
    double val  = *doocs_adapter1;
    double res1 = *doocs_adapter1 * 2;
    double res2 = *doocs_adapter1 * *doocs_adapter2;
    double res3 = *doocs_adapter1 * 2.5;

	doocs_adapter2->setWithoutCallback((*doocs_adapter1 * *doocs_adapter2) / 2.0);
    
    // -- then --
    BOOST_CHECK( val  == 5 );
    BOOST_CHECK( val  == *doocs_adapter1 );
    BOOST_CHECK( 5    == *doocs_adapter1 );
    BOOST_CHECK( res1 == 10 );
    BOOST_CHECK( res2 == 20 );
    BOOST_CHECK( res3 == 12.5 );

    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 10 );
}


BOOST_AUTO_TEST_CASE( conversion__callbacks_operation )
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(4);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    // -- when --
    double val  = *doocs_adapter1;
    double res1 = *doocs_adapter1 * 2;
    double res2 = *doocs_adapter1 * *doocs_adapter2;
    double res3 = *doocs_adapter1 * 2.5;                  val += res1 + res2 + res3;  // suppress -Wunused-variable

	doocs_adapter2->setWithoutCallback((*doocs_adapter1 * *doocs_adapter2) / 2.0);
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 10 );
    
    BOOST_CHECK( _get_cb_counter1        == 0 );    // make sure callbacks not called
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
}


BOOST_AUTO_TEST_CASE( conversion__callbacks_assignment ) // or rather for no assignment
{
    // -- given -- 
    reset_counters();

    doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
    doocs_adapter1->clearOnSetCallbackFunction();
    doocs_adapter2->clearOnGetCallbackFunction();
    doocs_adapter2->clearOnSetCallbackFunction();

    doocs_adapter1->setWithoutCallback(5);          // some init vals
    doocs_adapter2->setWithoutCallback(3);

    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
    
    doocs_adapter1->set(1);                         // do some random get/set() ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 1 );    // ... and check the callback operation so far
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 2 );
    
    // -- when --
    double val  = *doocs_adapter1;
    double res1 = *doocs_adapter1 * 2;
    double res2 = *doocs_adapter1 * *doocs_adapter2;
    double res3 = *doocs_adapter1 * 2.5;                  val += res1 + res2 + res3;  // suppress -Wunused-variable

	doocs_adapter2->setWithoutCallback((*doocs_adapter1 * *doocs_adapter2) / 2);
    
    // -- then --
    BOOST_CHECK( _get_cb_counter1        == 1 );    // counters state afterwards
    BOOST_CHECK( _set_cb_counter1        == 1 );
    BOOST_CHECK( _get_cb_counter2        == 2 );
    BOOST_CHECK( _set_cb_counter2        == 2 );    // <---

    doocs_adapter1->set(1);                         // do some random get/set() again ...
    doocs_adapter1->setWithoutCallback(1);
    doocs_adapter1->get();
    doocs_adapter1->getWithoutCallback();
    doocs_adapter2->set(1);
    doocs_adapter2->setWithoutCallback(1);
    doocs_adapter2->get();
    doocs_adapter2->getWithoutCallback();

    BOOST_CHECK( _get_cb_counter1        == 2 );    // ... and make sure the callback assignment remains unaltered
    BOOST_CHECK( _set_cb_counter1        == 2 );
    BOOST_CHECK( _get_cb_counter2        == 4 );
    BOOST_CHECK( _set_cb_counter2        == 4 );
}

BOOST_AUTO_TEST_SUITE_END()
