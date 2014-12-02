#define BOOST_TEST_MODULE callback_model_test
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "m4uD_type.hpp"

#include "D_float_mock.hpp"




// ============================================================================


struct CallbacksTestFixture {
    
    m4uD_type<float, D_float>                              * mydtype;


    unsigned int _get_cb_counter;
    unsigned int _set_cb_counter;
    unsigned int _set_cb_counter_equals;



            CallbacksTestFixture() : _get_cb_counter       (0),
                                     _set_cb_counter       (0),
                                     _set_cb_counter_equals(0)
            {
                mydtype       = new m4uD_type<float, D_float> ( NULL, NULL );
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


BOOST_FIXTURE_TEST_SUITE( test_callbacks, CallbacksTestFixture )

BOOST_AUTO_TEST_CASE( test_get_cb_count )
{
    reset_counters();

    BOOST_CHECK( _get_cb_counter        == 0 );
    
    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 1 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    mydtype->clearOnGetCallbackFunction();
    
    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 3 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 4 );


    mydtype->clearOnGetCallbackFunction();
    
    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 4 );

    mydtype      ->value();
    BOOST_CHECK( _get_cb_counter        == 4 );
}


BOOST_AUTO_TEST_CASE( test_set_cb_count )
{
    reset_counters();

    
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 1 );
    BOOST_CHECK( _set_cb_counter_equals == 1 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );


    mydtype->clearOnSetCallbackFunction();
    
    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );

    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 2 );
    BOOST_CHECK( _set_cb_counter_equals == 2 );


    mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 3 );
    BOOST_CHECK( _set_cb_counter_equals == 3 );

    mydtype      ->set_value(2);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 3 );


    mydtype->clearOnSetCallbackFunction();
    
    mydtype      ->set_value(1);
    BOOST_CHECK( _set_cb_counter        == 4 );
    BOOST_CHECK( _set_cb_counter_equals == 3 );
}

BOOST_AUTO_TEST_CASE( test_sync1 )
{
    reset_counters();

    
    BOOST_CHECK( mydtype      ->value() == 0 );


    mydtype      ->set_value(1);
    BOOST_CHECK( mydtype      ->value() == 1 );  // <---

    mydtype      ->set_value(0);
    BOOST_CHECK( mydtype      ->value() == 0 );  // <---

    mydtype->set_value(3);
    BOOST_CHECK( mydtype      ->value() == 3 );  // <---

    mydtype->set_value(0);
    BOOST_CHECK( mydtype      ->value() == 0 );  // <---
    

    mydtype      ->set_value(0);
}

BOOST_AUTO_TEST_SUITE_END()


// ============================================================================


BOOST_FIXTURE_TEST_SUITE( test_no_callbacks, CallbacksTestFixture )

BOOST_AUTO_TEST_CASE( test_get_nocb_count )
{
    reset_counters();

    BOOST_CHECK( _get_cb_counter        == 0 );
    

    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydtype->clearOnGetCallbackFunction();
    
    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydtype->setOnGetCallbackFunction(boost::bind (&CallbacksTestFixture::on_get_callback, this));
    
    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydtype->clearOnGetCallbackFunction();
    
    mydtype      ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );
}


BOOST_AUTO_TEST_CASE( test_set_nocb_count )
{
    reset_counters();

    
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
    

    mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    mydtype->clearOnSetCallbackFunction();
    
    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    mydtype->setOnSetCallbackFunction(boost::bind (&CallbacksTestFixture::on_set_callback, this, _1, _2));
    
    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );


    mydtype->clearOnSetCallbackFunction();
    
    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );

    mydtype      ->set_value_without_callback(1);
    BOOST_CHECK( _set_cb_counter        == 0 );
    BOOST_CHECK( _set_cb_counter_equals == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
