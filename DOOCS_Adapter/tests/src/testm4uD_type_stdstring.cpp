#define BOOST_TEST_MODULE test_string




#include "eq_fct.h"

#include "m4uD_type.hpp"




// ============================================================================


struct CallbacksTestFixture {
    
    m4uD_type<std::string, D_string>                              * mydtype;


    unsigned int _get_cb_counter;
    unsigned int _set_cb_counter;
    unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : _get_cb_counter       (0),
                                     _set_cb_counter       (0),
                                     _set_cb_counter_equals(0)
            {
                mydtype       = new m4uD_type<std::string, D_string> ( NULL, NULL );
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
    std::string on_get_callback ()                                               //~  < T () >
            {
                ++_get_cb_counter;
                return std::string("0");
            }
    void    on_set_callback (std::string const & newValue, std::string const & oldValue) //~  < void (T const &, T const & ) >
            {
                if ( newValue.compare(oldValue) == 0 ) ++_set_cb_counter_equals;
                ++_set_cb_counter;
            }
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;





BOOST_FIXTURE_TEST_SUITE( test_operation, CallbacksTestFixture ) // operation check: return values matter

	BOOST_AUTO_TEST_CASE( test_getset_nocb )
	{
		mydtype->set_value("1");
		BOOST_CHECK_EQUAL( mydtype->value(), "1" );  // <---

		mydtype->set_value("0");
		BOOST_CHECK_EQUAL( mydtype->value(), "0" );  // <---

		mydtype->set_value("3");
		BOOST_CHECK_EQUAL( mydtype->value(), "3" );  // <---

		mydtype->set_value("0");
		BOOST_CHECK_EQUAL( mydtype->value(), "0" );  // <---
	}


	BOOST_AUTO_TEST_CASE( test_getwcsetwc_nocb )
	{
		mydtype->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "1" );  // <---

		mydtype->set_value_without_callback("0");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "0" );  // <---

		mydtype->set_value_without_callback("3");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "3" );  // <---

		mydtype->set_value_without_callback("0");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "0" );  // <---
	}


	BOOST_AUTO_TEST_CASE( test_getset_cb )
	{
		mydtype->set_value("1");
		BOOST_CHECK_EQUAL( mydtype->value(), "1" );  // <---

		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		mydtype->set_value("2");
		BOOST_CHECK_EQUAL( mydtype->value(), "2" );  // <---

		mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		mydtype->set_value("3");
		BOOST_CHECK_EQUAL( mydtype->value(), "0" );  // <---

		mydtype->set_value("4");
		BOOST_CHECK_EQUAL( mydtype->value(), "0" );  // <---
	}


	BOOST_AUTO_TEST_CASE( test_getwcsetwc_cb )
	{
		mydtype->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "1" );  // <---

		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		mydtype->set_value_without_callback("2");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "2" );  // <---

		mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		mydtype->set_value_without_callback("3");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "3" );  // <---

		mydtype->set_value_without_callback("4");
		BOOST_CHECK_EQUAL( mydtype->value_without_callback(), "4" );  // <---
	}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_callbacks, CallbacksTestFixture ) // callback arbitrarily present: callback counters matter, return values don't

	BOOST_AUTO_TEST_CASE( test_get_cb_count )
	{
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );
		
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );
		

		mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 1 );
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 2 );


		mydtype->clearOnGetCallbackFunction();
		
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 2 );
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 2 );


		mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 3 );
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 4 );


		mydtype->clearOnGetCallbackFunction();
		
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 4 );
		mydtype      ->value();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 4 );
	}


	BOOST_AUTO_TEST_CASE( test_set_cb_count )
	{
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );
		

		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );

		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );
		

		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 1 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 1 );

		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 2 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 2 );


		mydtype->clearOnSetCallbackFunction();
		
		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 2 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 2 );

		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 2 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 2 );


		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 3 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 3 );

		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 4 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 4 );


		mydtype->clearOnSetCallbackFunction();
		
		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 4 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 4 );
	}

	BOOST_AUTO_TEST_CASE( test_set_cb_equals_count )
	{
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );
		

		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		mydtype      ->set_value("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 1 );

		mydtype      ->set_value("2");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 2 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );

		mydtype      ->set_value("2");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 3 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 1 );
	}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_no_callbacks, CallbacksTestFixture ) // callback absent: callback counters matter, return values don't

	BOOST_AUTO_TEST_CASE( test_get_nocb_count )
	{
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );
		

		mydtype      ->value_without_callback();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );
		

		mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		mydtype      ->value_without_callback();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );


		mydtype->clearOnGetCallbackFunction();
		
		mydtype      ->value_without_callback();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );


		mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
		
		mydtype      ->value_without_callback();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );


		mydtype->clearOnGetCallbackFunction();
		
		mydtype      ->value_without_callback();
		BOOST_CHECK_EQUAL( _get_cb_counter        , 0 );
	}


	BOOST_AUTO_TEST_CASE( test_set_nocb_count )
	{
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );
		

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );
		

		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );


		mydtype->clearOnSetCallbackFunction();
		
		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );


		mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
		
		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );


		mydtype->clearOnSetCallbackFunction();
		
		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );

		mydtype      ->set_value_without_callback("1");
		BOOST_CHECK_EQUAL( _set_cb_counter        , 0 );
		BOOST_CHECK_EQUAL( _set_cb_counter_equals , 0 );
	}

BOOST_AUTO_TEST_SUITE_END()
