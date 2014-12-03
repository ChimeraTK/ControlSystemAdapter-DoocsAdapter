#define BOOST_TEST_MODULE callback_model_test

#include <string>

#include "D_string_mock.hpp"

#include "m4uD_type.hpp"
#include "DOOCSProcessVariableAdapter.hpp"



struct CallbacksTestFixture {
	
    mtca4u::DOOCSPVAdapter<std::string, m4uD_type<std::string, D_string> > * doocs_adapter;
    m4uD_type<std::string, D_string>                                       * mydtype;


	unsigned int _get_cb_counter;
	unsigned int _set_cb_counter;
	unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : _get_cb_counter       (0),
                                     _set_cb_counter       (0),
                                     _set_cb_counter_equals(0)
            {
                mydtype       = new m4uD_type<std::string, D_string> ( NULL, NULL );
                doocs_adapter = new mtca4u::DOOCSPVAdapter<std::string, m4uD_type<std::string, D_string> > (mydtype);
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
	std::string	on_get_callback ()	                                             //~  < T () >
			{
				++_get_cb_counter;
				return std::string("0");
			}
	void	on_set_callback (std::string const & newValue, std::string const & oldValue) //~  < void (T const &, T const & ) >
			{
				if ( newValue.compare(oldValue) == 0 ) ++_set_cb_counter_equals;
				++_set_cb_counter;
			}
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


struct InterPVTestFixture {         // for testing interactions between two PVs,
                                    // like setting or assigning from a PV
    mtca4u::DOOCSPVAdapter<std::string, m4uD_type<std::string, D_string> > * doocs_adapter1;
    mtca4u::DOOCSPVAdapter<std::string, m4uD_type<std::string, D_string> > * doocs_adapter2;
    
    m4uD_type<std::string, D_string>                                  * mydtype1;
    m4uD_type<std::string, D_string>                                  * mydtype2;


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
                mydtype1       = new m4uD_type<std::string, D_string> ( NULL, NULL );
                mydtype2       = new m4uD_type<std::string, D_string> ( NULL, NULL );
                doocs_adapter1 = new mtca4u::DOOCSPVAdapter<std::string, m4uD_type<std::string, D_string> > (mydtype1);
                doocs_adapter2 = new mtca4u::DOOCSPVAdapter<std::string, m4uD_type<std::string, D_string> > (mydtype2);
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
	std::string  on_get_callback1 ()	                                              //~  < T () >
			{
				++_get_cb_counter1;
				return std::string("0");
			}
	void	on_set_callback1 (std::string const & newValue, std::string const & oldValue) //~  < void (T const &, T const & ) >
			{
				if ( newValue.compare(oldValue) == 0 ) ++_set_cb_counter_equals1;
				++_set_cb_counter1;
			}
            // set 2 (NOTE: different treating of counters!)
	std::string	on_get_callback2 ()	                                              //~  < T () >
			{
				_get_cb_counter2 += 2;
				return std::string("0");
			}
	void	on_set_callback2 (std::string const & newValue, std::string const & oldValue) //~  < void (T const &, T const & ) >
			{
				if ( newValue.compare(oldValue) == 0 ) _set_cb_counter_equals2 += 2;
				_set_cb_counter2 += 2;
			}
};


// ============================================================================
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ============================================================================


#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;





BOOST_FIXTURE_TEST_SUITE( test_operation, CallbacksTestFixture ) // operation check: return values matter

	BOOST_AUTO_TEST_CASE( test_getset_nocb )
	{
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("<empty>") );
		BOOST_CHECK_EQUAL( mydtype      ->value(), "<empty>" );


		doocs_adapter->set(std::string("1"));
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("1") );
		BOOST_CHECK_EQUAL( mydtype      ->value(), "1" );  // <---

		doocs_adapter->set(std::string("0"));
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("0") );
		BOOST_CHECK_EQUAL( mydtype      ->value(), "0" );  // <---

		mydtype->set_value("3");
		BOOST_CHECK_EQUAL( mydtype      ->value(), "3" );
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("3") );  // <---

		mydtype->set_value("0");
		BOOST_CHECK_EQUAL( mydtype      ->value(), "0" );
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("0") );  // <---
	}


	BOOST_AUTO_TEST_CASE( test_getwcsetwc_nocb )
	{
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("<empty>") );
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "<empty>" );


		doocs_adapter->setWithoutCallback(std::string("1"));
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("1") );
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "1" );  // <---

		doocs_adapter->setWithoutCallback(std::string("0"));
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("0") );
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "0" );  // <---

		mydtype->set_value_without_callback("3");
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "3" );
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("3") );  // <---

		mydtype->set_value_without_callback("0");
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "0" );
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("0") );  // <---
	}


	BOOST_AUTO_TEST_CASE( test_getset_cb )
	{
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("<empty>") );
		BOOST_CHECK_EQUAL( mydtype      ->value(), "<empty>" );


		doocs_adapter->set(std::string("1"));
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("1") );
		BOOST_CHECK_EQUAL( mydtype      ->value(), "1" );  // <---

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		doocs_adapter->set(std::string("2"));
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("2") );
		BOOST_CHECK_EQUAL( mydtype      ->value(), "2" );  // <---

		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		mydtype->set_value("3");
		BOOST_CHECK_EQUAL( mydtype      ->value(), "0" );
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("0") );  // <---

		mydtype->set_value("4");
		BOOST_CHECK_EQUAL( mydtype      ->value(), "0" );
		BOOST_CHECK_EQUAL( doocs_adapter->get  (), std::string("0") );  // <---
	}


	BOOST_AUTO_TEST_CASE( test_getwcsetwc_cb )
	{
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("<empty>") );
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "<empty>" );


		doocs_adapter->setWithoutCallback(std::string("1"));
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("1") );
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "1" );  // <---

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		doocs_adapter->setWithoutCallback(std::string("2"));
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("2") );
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "2" );  // <---

		doocs_adapter->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		mydtype->set_value_without_callback("3");
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "3" );
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("3") );  // <---

		mydtype->set_value_without_callback("4");
		BOOST_CHECK_EQUAL( mydtype      ->value_without_callback(), "4" );
		BOOST_CHECK_EQUAL( doocs_adapter->getWithoutCallback    (), std::string("4") );  // <---
	}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_callbacks, CallbacksTestFixture ) // callback arbitrarily present: callback counters matter, return values don't

	BOOST_AUTO_TEST_CASE( test_get_cb_count )
	{
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
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->set(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		mydtype      ->set_value("1");
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->set(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 1 );
		BOOST_CHECK( _set_cb_counter_equals == 1 );

		mydtype      ->set_value("1");
		BOOST_CHECK( _set_cb_counter        == 2 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );


		doocs_adapter->clearOnSetCallbackFunction();
		
		doocs_adapter->set(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 2 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );

		mydtype      ->set_value("1");
		BOOST_CHECK( _set_cb_counter        == 2 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );


		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->set(std::string("2"));
		BOOST_CHECK( _set_cb_counter        == 3 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );

		mydtype      ->set_value("1");
		BOOST_CHECK( _set_cb_counter        == 4 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );


		mydtype->clearOnSetCallbackFunction();
		
		doocs_adapter->set(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 4 );
		BOOST_CHECK( _set_cb_counter_equals == 2 );

		mydtype      ->set_value("1");
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
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->setWithoutCallback(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );
		

		doocs_adapter->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->setWithoutCallback(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );


		doocs_adapter->clearOnSetCallbackFunction();
		
		doocs_adapter->setWithoutCallback(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );


		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		doocs_adapter->setWithoutCallback(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );


		mydtype->clearOnSetCallbackFunction();
		
		doocs_adapter->setWithoutCallback(std::string("1"));
		BOOST_CHECK( _set_cb_counter        == 0 );
		BOOST_CHECK( _set_cb_counter_equals == 0 );

		mydtype      ->set_value_without_callback("1");
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
		doocs_adapter1->setWithoutCallback(std::string("5"));
		doocs_adapter2->setWithoutCallback(std::string("0"));

		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("0") );
		
		// -- when --
		doocs_adapter2->set(*doocs_adapter1);
		
		// -- then --
		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
	}


	BOOST_AUTO_TEST_CASE( set_from_other_pv__callbacks_operation )
	{
		// -- given -- 
		doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
		doocs_adapter1->clearOnSetCallbackFunction();
		doocs_adapter2->clearOnGetCallbackFunction();
		doocs_adapter2->clearOnSetCallbackFunction();

		doocs_adapter1->setWithoutCallback(std::string("5"));          // some init vals
		doocs_adapter2->setWithoutCallback(std::string("0"));

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
		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
		
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

		doocs_adapter1->setWithoutCallback(std::string("5"));          // some init vals
		doocs_adapter2->setWithoutCallback(std::string("0"));

		BOOST_CHECK( _get_cb_counter1        == 0 );
		BOOST_CHECK( _set_cb_counter1        == 0 );
		BOOST_CHECK( _get_cb_counter2        == 0 );
		BOOST_CHECK( _set_cb_counter2        == 0 );
		
		doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
		doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
		doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
		doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
		
		doocs_adapter1->set(std::string("1"));                         // do some random get/set() ...
		doocs_adapter1->setWithoutCallback(std::string("1"));
		doocs_adapter1->get();
		doocs_adapter1->getWithoutCallback();
		doocs_adapter2->set(std::string("1"));
		doocs_adapter2->setWithoutCallback(std::string("1"));
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

		doocs_adapter1->set(std::string("1"));                         // do some random get/set() again ...
		doocs_adapter1->setWithoutCallback(std::string("1"));
		doocs_adapter1->get();
		doocs_adapter1->getWithoutCallback();
		doocs_adapter2->set(std::string("1"));
		doocs_adapter2->setWithoutCallback(std::string("1"));
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
		doocs_adapter1->setWithoutCallback(std::string("5"));
		doocs_adapter2->setWithoutCallback(std::string("0"));

		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("0") );
		
		// -- when --
		doocs_adapter2->setWithoutCallback(*doocs_adapter1);
		
		// -- then --
		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
	}


	BOOST_AUTO_TEST_CASE( setWithoutCallback_from_other_pv__callbacks_operation )
	{
		// -- given -- 
		doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
		doocs_adapter1->clearOnSetCallbackFunction();
		doocs_adapter2->clearOnGetCallbackFunction();
		doocs_adapter2->clearOnSetCallbackFunction();

		doocs_adapter1->setWithoutCallback(std::string("5"));          // some init vals
		doocs_adapter2->setWithoutCallback(std::string("0"));

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
		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
		
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

		doocs_adapter1->setWithoutCallback(std::string("5"));          // some init vals
		doocs_adapter2->setWithoutCallback(std::string("0"));

		BOOST_CHECK( _get_cb_counter1        == 0 );
		BOOST_CHECK( _set_cb_counter1        == 0 );
		BOOST_CHECK( _get_cb_counter2        == 0 );
		BOOST_CHECK( _set_cb_counter2        == 0 );
		
		doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
		doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
		doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
		doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
		
		doocs_adapter1->set(std::string("1"));                         // do some random get/set() ...
		doocs_adapter1->setWithoutCallback(std::string("1"));
		doocs_adapter1->get();
		doocs_adapter1->getWithoutCallback();
		doocs_adapter2->set(std::string("1"));
		doocs_adapter2->setWithoutCallback(std::string("1"));
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

		doocs_adapter1->set(std::string("1"));                         // do some random get/set() again ...
		doocs_adapter1->setWithoutCallback(std::string("1"));
		doocs_adapter1->get();
		doocs_adapter1->getWithoutCallback();
		doocs_adapter2->set(std::string("1"));
		doocs_adapter2->setWithoutCallback(std::string("1"));
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
		doocs_adapter1->setWithoutCallback(std::string("5"));
		doocs_adapter2->setWithoutCallback(std::string("0"));

		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("0") );
		
		// -- when --
		*doocs_adapter2 = *doocs_adapter1;
		
		// -- then --
		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
	}


	BOOST_AUTO_TEST_CASE( assign_from_other_pv__callbacks_operation )
	{
		// -- given -- 
		doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
		doocs_adapter1->clearOnSetCallbackFunction();
		doocs_adapter2->clearOnGetCallbackFunction();
		doocs_adapter2->clearOnSetCallbackFunction();

		doocs_adapter1->setWithoutCallback(std::string("5"));          // some init vals
		doocs_adapter2->setWithoutCallback(std::string("0"));

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
		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
		
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

		doocs_adapter1->setWithoutCallback(std::string("5"));          // some init vals
		doocs_adapter2->setWithoutCallback(std::string("0"));

		BOOST_CHECK( _get_cb_counter1        == 0 );
		BOOST_CHECK( _set_cb_counter1        == 0 );
		BOOST_CHECK( _get_cb_counter2        == 0 );
		BOOST_CHECK( _set_cb_counter2        == 0 );
		
		doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
		doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
		doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2));
		doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
		
		doocs_adapter1->set(std::string("1"));                         // do some random get/set() ...
		doocs_adapter1->setWithoutCallback(std::string("1"));
		doocs_adapter1->get();
		doocs_adapter1->getWithoutCallback();
		doocs_adapter2->set(std::string("1"));
		doocs_adapter2->setWithoutCallback(std::string("1"));
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

		doocs_adapter1->set(std::string("1"));                         // do some random get/set() again ...
		doocs_adapter1->setWithoutCallback(std::string("1"));
		doocs_adapter1->get();
		doocs_adapter1->getWithoutCallback();
		doocs_adapter2->set(std::string("1"));
		doocs_adapter2->setWithoutCallback(std::string("1"));
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
		doocs_adapter2->setWithoutCallback(std::string("0"));

		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("0") );
		
		// -- when --
		*doocs_adapter2 = std::string("5");
		
		// -- then --
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
	}


	BOOST_AUTO_TEST_CASE( assign_from_primtype__callbacks_operation )
	{
		// -- given -- 
		doocs_adapter2->clearOnGetCallbackFunction();   // make sure no callbacks
		doocs_adapter2->clearOnSetCallbackFunction();

		doocs_adapter2->setWithoutCallback(std::string("0"));          // some init val

		BOOST_CHECK( _get_cb_counter2        == 0 );
		BOOST_CHECK( _set_cb_counter2        == 0 );
		
		doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2)); // prime callbacks
		doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
		
		// -- when --
		*doocs_adapter2 = std::string("5");
		
		// -- then --
		BOOST_CHECK_EQUAL( doocs_adapter2->getWithoutCallback(), std::string("5") );
		
		BOOST_CHECK( _get_cb_counter2        == 0 );    // make sure callbacks not called
		BOOST_CHECK( _set_cb_counter2        == 0 );
	}


	BOOST_AUTO_TEST_CASE( assign_from_primtype__callbacks_assignment ) // or rather for no assignment
	{
		// -- given -- 
		doocs_adapter2->clearOnGetCallbackFunction();   // make sure no callbacks
		doocs_adapter2->clearOnSetCallbackFunction();

		doocs_adapter2->setWithoutCallback(std::string("0"));          // some init vals

		BOOST_CHECK( _get_cb_counter2        == 0 );
		BOOST_CHECK( _set_cb_counter2        == 0 );
		
		doocs_adapter2->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback2, this, _1, _2)); // prime callbacks
		doocs_adapter2->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback2, this));
		
		doocs_adapter2->set(std::string("1"));                         // do some random get/set() ...
		doocs_adapter2->setWithoutCallback(std::string("1"));
		doocs_adapter2->get();
		doocs_adapter2->getWithoutCallback();

		BOOST_CHECK( _get_cb_counter2        == 2 );    // ... and check the callback operation so far
		BOOST_CHECK( _set_cb_counter2        == 2 );
		
		// -- when --
		*doocs_adapter2 = std::string("5");
		
		// -- then --
		BOOST_CHECK( _get_cb_counter2        == 2 );    // counters state after set()
		BOOST_CHECK( _set_cb_counter2        == 2 );    // <---

		doocs_adapter2->set(std::string("1"));                         // do some random get/set() again ...
		doocs_adapter2->setWithoutCallback(std::string("1"));
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
		doocs_adapter1->setWithoutCallback(std::string("5"));

		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
        
        std::string teststr = "00";
		
		// -- when --
        std::string val  = *doocs_adapter1;
        std::string deststr = teststr+val;

		// -- then --
        BOOST_CHECK_EQUAL( val, std::string("5") );
        BOOST_CHECK_EQUAL( deststr, std::string("005") );
        //~ BOOST_CHECK_EQUAL( *doocs_adapter1, val );  // won't work. doocs_adapter1 does not overload operator==

		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
	}


	BOOST_AUTO_TEST_CASE( conversion__callbacks_operation )
	{
		// -- given -- 
		doocs_adapter1->clearOnGetCallbackFunction();   // make sure no callbacks
		doocs_adapter1->clearOnSetCallbackFunction();

		doocs_adapter1->setWithoutCallback(std::string("5"));          // some init vals

		BOOST_CHECK( _get_cb_counter1        == 0 );
		BOOST_CHECK( _set_cb_counter1        == 0 );
		BOOST_CHECK( _get_cb_counter2        == 0 );
		BOOST_CHECK( _set_cb_counter2        == 0 );
		
		doocs_adapter1->setOnSetCallbackFunction(boost::bind (&InterPVTestFixture::on_set_callback1, this, _1, _2)); // prime callbacks
		doocs_adapter1->setOnGetCallbackFunction(boost::bind (&InterPVTestFixture::on_get_callback1, this));
		
		// -- when --
        std::string val  = *doocs_adapter1;            
        int suppress = 0; suppress += val.length();  // suppress -Wunused-variable

		// -- then --
		BOOST_CHECK_EQUAL( doocs_adapter1->getWithoutCallback(), std::string("5") );
		
		BOOST_CHECK( _get_cb_counter1        == 0 );    // make sure callbacks not called
		BOOST_CHECK( _set_cb_counter1        == 0 );
		BOOST_CHECK( _get_cb_counter2        == 0 );
		BOOST_CHECK( _set_cb_counter2        == 0 );
	}

BOOST_AUTO_TEST_SUITE_END()
