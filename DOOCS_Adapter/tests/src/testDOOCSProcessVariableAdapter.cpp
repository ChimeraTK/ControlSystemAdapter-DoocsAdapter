#define BOOST_TEST_MODULE callback_model_test
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "D_int_mock.hpp"
#include "myD_int.hpp"
#include "DOOCSProcessVariableAdapter.hpp"






// ============================================================================


struct CbTestFixture {
	
    DOOCSPVAdapter * doocs_adapter;
    myD_int        * mydint;


	unsigned int _get_cb_counter;
	unsigned int _set_cb_counter;
	unsigned int _set_cb_counter_equals;



            CbTestFixture() : _get_cb_counter(0),
                              _set_cb_counter(0),
                              _set_cb_counter_equals(0)
            {
                mydint        = new myD_int( NULL, NULL );
                doocs_adapter = new DOOCSPVAdapter(mydint);
            }
    
            ~CbTestFixture()
            {
                delete doocs_adapter;
                delete mydint;
            }
    
    	

	void	reset_fixture()
			{
				_get_cb_counter = 0;
				_set_cb_counter = 0;
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




BOOST_FIXTURE_TEST_SUITE( test_cb, CbTestFixture )

BOOST_AUTO_TEST_CASE( test_get_cb_count )
{
    reset_fixture();

    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->setOnGetCallbackFunction(boost::bind (&CbTestFixture::on_get_callback, this));
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 1 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    doocs_adapter->clearOnGetCallbackFunction();
    
    doocs_adapter->get  ();
    BOOST_CHECK( _get_cb_counter        == 2 );

    mydint       ->value();
    BOOST_CHECK( _get_cb_counter        == 2 );


    mydint->setOnGetCallbackFunction(boost::bind (&CbTestFixture::on_get_callback, this));
    
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
    

    doocs_adapter->setOnSetCallbackFunction(boost::bind (&CbTestFixture::on_set_callback, this, _1, _2));
    
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


    mydint->setOnSetCallbackFunction(boost::bind (&CbTestFixture::on_set_callback, this, _1, _2));
    
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


BOOST_FIXTURE_TEST_SUITE( test_no_cb, CbTestFixture )

BOOST_AUTO_TEST_CASE( test_get_nocb_count )
{
    reset_fixture();

    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );
    

    doocs_adapter->setOnGetCallbackFunction(boost::bind (&CbTestFixture::on_get_callback, this));
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    doocs_adapter->clearOnGetCallbackFunction();
    
    doocs_adapter->getWithoutCallback();
    BOOST_CHECK( _get_cb_counter        == 0 );

    mydint       ->value_without_callback();
    BOOST_CHECK( _get_cb_counter        == 0 );


    mydint->setOnGetCallbackFunction(boost::bind (&CbTestFixture::on_get_callback, this));
    
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
    

    doocs_adapter->setOnSetCallbackFunction(boost::bind (&CbTestFixture::on_set_callback, this, _1, _2));
    
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


    mydint->setOnSetCallbackFunction(boost::bind (&CbTestFixture::on_set_callback, this, _1, _2));
    
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

