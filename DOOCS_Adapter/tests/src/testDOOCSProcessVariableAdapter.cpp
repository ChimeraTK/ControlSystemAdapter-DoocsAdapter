#define BOOST_TEST_MODULE callback_model_test
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "D_int_mock.hpp"
#include "myD_int.hpp"
#include "DOOCSProcessVariableAdapter.hpp"






// ============================================================================


struct CallbacksTestFixture {
	
    DOOCSPVAdapter * doocs_adapter;
    myD_int        * mydint;


	unsigned int _get_cb_counter;
	unsigned int _set_cb_counter;
	unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : _get_cb_counter(0),
                                     _set_cb_counter(0),
                                     _set_cb_counter_equals(0)
            {
                mydint        = new myD_int( NULL, NULL );
                doocs_adapter = new DOOCSPVAdapter(mydint);
            }
    
            ~CallbacksTestFixture()
            {
                delete doocs_adapter;
                delete mydint;
            }
    
    	

	void	reset_fixture()
			{
				_get_cb_counter        = 0;
				_set_cb_counter        = 0;
				_set_cb_counter_equals = 0;
			}
	

            // callbacks
	int		on_get_callback ()	                                         //~  < int () >
			{
				++_get_cb_counter;
				return 0;
			}
	void	on_set_callback (int const & newValue, int const & oldValue) //~  < void (int const &, int const & ) >
			{
				if (newValue == oldValue) ++_set_cb_counter_equals;
				++_set_cb_counter;
			}
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


BOOST_FIXTURE_TEST_SUITE( test_callbacks, CallbacksTestFixture )

BOOST_AUTO_TEST_CASE( test_get_cb_count )
{
    reset_fixture();

    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 1 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    doocs_adapter->clearOnGetCallbackFunction();
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 2 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    mydint->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 3 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 4 );


    mydint->clearOnGetCallbackFunction();
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 4 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 4 );
}


BOOST_AUTO_TEST_CASE( test_set_cb_count )
{
    reset_fixture();

    
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydint       ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 1 );
    BOOST_CHECK( _set_cb_counter_equals == 1 );

    mydint       ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );


    doocs_adapter->clearOnSetCallbackFunction();
    
    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );

    mydint       ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );


    mydint->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 3 );
    BOOST_CHECK( _set_cb_counter_equals == 3 );

    mydint       ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 4 );


    mydint->clearOnSetCallbackFunction();
    
    doocs_adapter->set(1);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 4 );

    mydint       ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 4 );
}

BOOST_AUTO_TEST_CASE( test_sync1 )
{
    reset_fixture();

    
    BOOST_CHECK( doocs_adapter->get  () == 0 );
    BOOST_CHECK( mydint       ->value() == 0 );


    doocs_adapter->set(1);
    BOOST_CHECK( doocs_adapter->get  () == 1 );
    BOOST_CHECK( mydint       ->value() == 1 );  // <---

    doocs_adapter->set(0);
    BOOST_CHECK( doocs_adapter->get  () == 0 );
    BOOST_CHECK( mydint       ->value() == 0 );  // <---

    mydint->set_value(3);
    BOOST_CHECK( mydint       ->value() == 3 );
    BOOST_CHECK( doocs_adapter->get  () == 3 );  // <---

    mydint->set_value(0);
    BOOST_CHECK( mydint       ->value() == 0 );
    BOOST_CHECK( doocs_adapter->get  () == 0 );  // <---
	

	doocs_adapter->set(0);
}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_no_callbacks, CallbacksTestFixture )

BOOST_AUTO_TEST_CASE( test_get_nocb_count )
{
    reset_fixture();

    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    doocs_adapter->clearOnGetCallbackFunction();
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydint->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydint->clearOnGetCallbackFunction();
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );
}


BOOST_AUTO_TEST_CASE( test_set_nocb_count )
{
    reset_fixture();

    
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydint       ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydint       ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    doocs_adapter->clearOnSetCallbackFunction();
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydint       ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    mydint->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydint       ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    mydint->clearOnSetCallbackFunction();
    
    doocs_adapter->setWithoutCallback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydint       ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


struct SetFromOtherPVTestFixture {
	
    DOOCSPVAdapter * doocs_adapter1;
    DOOCSPVAdapter * doocs_adapter2;
    
    myD_int        * mydint1;
    myD_int        * mydint2;


	unsigned int _get_cb_counter1;
	unsigned int _set_cb_counter1;
	unsigned int _set_cb_counter_equals1;
	unsigned int _get_cb_counter2;
	unsigned int _set_cb_counter2;
	unsigned int _set_cb_counter_equals2;



            SetFromOtherPVTestFixture() : _get_cb_counter1(0),
                                          _set_cb_counter1(0),
                                          _set_cb_counter_equals1(0),
                                          _get_cb_counter2(0),
                                          _set_cb_counter2(0),
                                          _set_cb_counter_equals2(0)
            {
                mydint1        = new myD_int( NULL, NULL );
                mydint2        = new myD_int( NULL, NULL );
                doocs_adapter1 = new DOOCSPVAdapter(mydint1);
                doocs_adapter2 = new DOOCSPVAdapter(mydint2);
            }
    
            ~SetFromOtherPVTestFixture()
            {
                delete doocs_adapter1;
                delete doocs_adapter2;
                delete mydint1;
                delete mydint2;
            }
    
    	

	void	reset_fixture()
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
	int		on_get_callback1 ()	                                         //~  < int () >
			{
				++_get_cb_counter1;
				return 0;
			}
	void	on_set_callback1 (int const & newValue, int const & oldValue) //~  < void (int const &, int const & ) >
			{
				if (newValue == oldValue) ++_set_cb_counter_equals1;
				++_set_cb_counter1;
			}
            // set 2 (NOTE: different treating of counters)
	int		on_get_callback2 ()	                                         //~  < int () >
			{
				_get_cb_counter2 += 2;
				return 0;
			}
	void	on_set_callback2 (int const & newValue, int const & oldValue) //~  < void (int const &, int const & ) >
			{
				if (newValue == oldValue) _set_cb_counter_equals2 += 2;
				_set_cb_counter2 += 2;
			}
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


BOOST_FIXTURE_TEST_SUITE( set_from_other_pv, SetFromOtherPVTestFixture )

BOOST_AUTO_TEST_CASE( set_from_other_pv__set_checking )
{
    // -- given -- 
    reset_fixture();
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
    reset_fixture();

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
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback2, this));
    
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
    reset_fixture();

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
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback2, this));
    
    doocs_adapter1->set(1);                         // do some random get/set(int) ...
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

    doocs_adapter1->set(1);                         // do some random get/set(int) again ...
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

BOOST_FIXTURE_TEST_SUITE( setWithoutCallback_from_other_pv, SetFromOtherPVTestFixture )

BOOST_AUTO_TEST_CASE( setWithoutCallback_from_other_pv__set_checking )
{
    // -- given -- 
    reset_fixture();
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
    reset_fixture();

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
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback2, this));
    
    // -- when --
    doocs_adapter2->setWithoutCallback(*doocs_adapter1);
    
    // -- then --
    BOOST_CHECK( doocs_adapter1->getWithoutCallback() == 5 );
    BOOST_CHECK( doocs_adapter2->getWithoutCallback() == 5 );
    
    BOOST_CHECK( _get_cb_counter1        == 0 );
    BOOST_CHECK( _set_cb_counter1        == 0 );
    BOOST_CHECK( _get_cb_counter2        == 0 );
    BOOST_CHECK( _set_cb_counter2        == 0 );
}


BOOST_AUTO_TEST_CASE( setWithoutCallback_from_other_pv__callbacks_assignment ) // or rather for no assignment
{
    // -- given -- 
    reset_fixture();

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
    
    doocs_adapter1->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
    doocs_adapter1->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback1, this));
    doocs_adapter2->setOnSetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_set_callback2, this, _1, _2));
    doocs_adapter2->setOnGetCallbackFunction(boost::bind (&SetFromOtherPVTestFixture::on_get_callback2, this));
    
    doocs_adapter1->set(1);                         // do some random get/set(int) ...
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

    doocs_adapter1->set(1);                         // do some random get/set(int) again ...
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
