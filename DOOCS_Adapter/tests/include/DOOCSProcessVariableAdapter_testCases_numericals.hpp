#ifndef __DOOCSProcessVariableAdapter_testCases_numericals__
#define __DOOCSProcessVariableAdapter_testCases_numericals__

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;



// for int, double, float


BOOST_FIXTURE_TEST_SUITE( test_operation, CallbacksTestFixture ) // operation check: return values matter

	BOOST_AUTO_TEST_CASE( test_getset_nocb )
	{
		BOOST_CHECK( doocs_adapter->get  () == 0 );


		doocs_adapter->set(1);
		BOOST_CHECK( doocs_adapter->get  () == 1 );

		doocs_adapter->set(0);
		BOOST_CHECK( doocs_adapter->get  () == 0 );
	}


	BOOST_AUTO_TEST_CASE( test_getwcsetwc_nocb )
	{
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 0 );


		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 1 );

		doocs_adapter->setWithoutCallback(0);
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 0 );
	}


	BOOST_AUTO_TEST_CASE( test_getset_cb )
	{
		BOOST_CHECK( doocs_adapter->get  () == 0 );


		doocs_adapter->set(1);
		BOOST_CHECK( doocs_adapter->get  () == 1 );

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		doocs_adapter->set(2);
		BOOST_CHECK( doocs_adapter->get  () == 2 );

		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		doocs_adapter->set(3);
		BOOST_CHECK( doocs_adapter->get  () == 0 );  // <---

		doocs_adapter->set(4);
		BOOST_CHECK( doocs_adapter->get  () == 0 );  // <---
	}


	BOOST_AUTO_TEST_CASE( test_getwcsetwc_cb )
	{
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 0 );


		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 1 );

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		doocs_adapter->setWithoutCallback(2);
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 2 );

		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		doocs_adapter->setWithoutCallback(3);
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 3 );  // <---

		doocs_adapter->setWithoutCallback(4);
		BOOST_CHECK( doocs_adapter->getWithoutCallback    () == 4 );  // <---
	}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_callbacks, CallbacksTestFixture ) // callback arbitrarily present: callback counters matter, return values don't

	BOOST_AUTO_TEST_CASE( test_get_cb_count )
	{
		BOOST_CHECK( _get_cb_counter        == 0 );
		

		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 0 );
		

		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 1 );
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 2 );


		doocs_adapter->clearOnGetCallbackFunction();
		
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 2 );
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 2 );


		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 3 );
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 4 );


		doocs_adapter->clearOnGetCallbackFunction();
		
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 4 );
		doocs_adapter->get  ();
		BOOST_CHECK( _get_cb_counter        == 4 );
	}


	BOOST_AUTO_TEST_CASE( test_set_cb_count )
	{
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 1 );
		BOOST_CHECK( _set_cb_counter_equals == 1 );

		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 2 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );


		doocs_adapter->clearOnSetCallbackFunction();
		
		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 2 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );

		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 2 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );


		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->set(2);
		BOOST_CHECK( _set_cb_counter        == 3 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );

		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 4 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );


		doocs_adapter->clearOnSetCallbackFunction();
		
		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 4 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );

		doocs_adapter->set(1);
		BOOST_CHECK( _set_cb_counter        == 4 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );
	}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_no_callbacks, CallbacksTestFixture ) // callback arbitrarily present: callback counters matter, return values don't

	BOOST_AUTO_TEST_CASE( test_get_nocb_count )
	{
		BOOST_CHECK( _get_cb_counter        == 0 );
		

		doocs_adapter->getWithoutCallback();
		BOOST_CHECK( _get_cb_counter        == 0 );
		

		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		doocs_adapter->getWithoutCallback();
		BOOST_CHECK( _get_cb_counter        == 0 );


		doocs_adapter->clearOnGetCallbackFunction();
		
		doocs_adapter->getWithoutCallback();
		BOOST_CHECK( _get_cb_counter        == 0 );


		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		doocs_adapter->getWithoutCallback();
		BOOST_CHECK( _get_cb_counter        == 0 );


		doocs_adapter->clearOnGetCallbackFunction();
		
		doocs_adapter->getWithoutCallback();
		BOOST_CHECK( _get_cb_counter        == 0 );
	}


	BOOST_AUTO_TEST_CASE( test_set_nocb_count )
	{
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );


		doocs_adapter->clearOnSetCallbackFunction();
		
		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );


		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );


		doocs_adapter->clearOnSetCallbackFunction();
		
		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		doocs_adapter->setWithoutCallback(1);
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
	}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ============================================================================


BOOST_FIXTURE_TEST_SUITE( set_from_other_pv, InterPVTestFixture )

	BOOST_AUTO_TEST_CASE( set_from_other_pv__set_checking )
	{
		// -- given -- 
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




#endif /* __DOOCSProcessVariableAdapter_testCases_numericals__ */

