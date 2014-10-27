#define BOOST_TEST_MODULE callback_model_test
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "DOOCSProcessVariableAdapter.hpp"
#include "myD_int_mock.hpp"





struct DPVATestFixture {
	
	unsigned int _get_cb_counter;
	unsigned int _set_cb_counter;
	unsigned int _set_cb_counter_equals;
	
	int _cbTestVar;		// callback functionality test variable

    DOOCSPVAdapter * doocs_adapter;

	DPVATestFixture() : _get_cb_counter(0), _set_cb_counter(0), _cbTestVar(0), _set_cb_counter_equals(0) {
        doocs_adapter = new DOOCSPVAdapter();
    }
	~DPVATestFixture() {
        delete doocs_adapter;
    }
	
	void	reset_fixture()
			{
				_get_cb_counter = 0;
				_set_cb_counter = 0;
				_cbTestVar = 0;
			}
	

	int		on_get_callback ()
			{
				++_get_cb_counter;
				return _cbTestVar;
			}

	void	on_set_callback (int const & newValue, int const & oldValue)
			{
				if (newValue == oldValue) ++_set_cb_counter_equals;
				_cbTestVar = newValue;
				++_set_cb_counter;
			}
};




BOOST_FIXTURE_TEST_SUITE( test_callbacks, DPVATestFixture )

BOOST_AUTO_TEST_CASE( test_get )
{
	doocs_adapter->setOnGetCallbackFunction(boost::bind (&DPVATestFixture::on_get_callback, this));
	
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );

	_cbTestVar = 5;
	
    BOOST_CHECK( doocs_adapter->get() == 5 );
    BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _get_cb_counter == 1 );

    BOOST_CHECK( doocs_adapter->get() == 5 );
    BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _get_cb_counter == 2 );

	_cbTestVar = 10;
	
    BOOST_CHECK( doocs_adapter->get() == 10 );
    BOOST_CHECK( _cbTestVar      == 10 );
	BOOST_CHECK( _get_cb_counter == 3 );


	doocs_adapter->clearOnGetCallbackFunction();

    BOOST_CHECK( doocs_adapter->get() == 10 );
    BOOST_CHECK( _cbTestVar      == 10 );
	BOOST_CHECK( _get_cb_counter == 3 );
	
    
	doocs_adapter->set(0);
	reset_fixture();
}


BOOST_AUTO_TEST_CASE( test_set )
{
	doocs_adapter->setOnSetCallbackFunction(boost::bind (&DPVATestFixture::on_set_callback, this, _1, _2));
	
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );
	
	doocs_adapter->set(5);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 1 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter->set(5);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 2 );
	BOOST_CHECK( _set_cb_counter_equals == 1 );


	doocs_adapter->clearOnSetCallbackFunction();

	doocs_adapter->set(5);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 2 );
	BOOST_CHECK( _set_cb_counter_equals == 1 );

	doocs_adapter->set(15);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 2 );
	BOOST_CHECK( _set_cb_counter_equals == 1 );

	
	doocs_adapter->set(0);
	reset_fixture();
}

BOOST_AUTO_TEST_SUITE_END()




BOOST_FIXTURE_TEST_SUITE( test_no_callbacks, DPVATestFixture )

BOOST_AUTO_TEST_CASE( test_setwithoutcallback_getwithoutcallback )
{
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter->setWithoutCallback(5);
	
    BOOST_CHECK( doocs_adapter->getWithoutCallback() == 5 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter->setWithoutCallback(10);
	
    BOOST_CHECK( doocs_adapter->getWithoutCallback() == 10 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	
	doocs_adapter->set(0);
	reset_fixture();
}


BOOST_AUTO_TEST_CASE( test_set_get__no_callbacks )
{
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter->set(5);
	
    BOOST_CHECK( doocs_adapter->get() == 5 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter->set(10);
	
    BOOST_CHECK( doocs_adapter->get() == 10 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	
	doocs_adapter->set(0);
	reset_fixture();
}

BOOST_AUTO_TEST_SUITE_END()




// ============================================================================


struct CbSyncTestFixture {
	
    DOOCSPVAdapter * doocs_adapter;
    myD_int        * mydint;


	CbSyncTestFixture() {
        doocs_adapter = new DOOCSPVAdapter();
        mydint        = new myD_int( NULL, NULL );
        mydint->init (doocs_adapter);
    }
    
	~CbSyncTestFixture() {
        delete doocs_adapter;
        delete mydint;
    }

};




BOOST_FIXTURE_TEST_SUITE( test_sync, CbSyncTestFixture )

BOOST_AUTO_TEST_CASE( test_accesors_count )
{   // values are suitable to the model where both entities keep a value copy of their own
    
    doocs_adapter->get  ();  // "reset"
    mydint       ->value();

    BOOST_CHECK( mydint->__value_f_call_counter           == 1 );
    BOOST_CHECK( mydint->__set_value_f_call_counter       == 0 );
    BOOST_CHECK( mydint->__on_get_callback_f_call_counter == 0 );
    BOOST_CHECK( mydint->__on_set_callback_f_call_counter == 0 );
    

    doocs_adapter->set(1);
    doocs_adapter->get  ();
    mydint       ->value();

    BOOST_CHECK( mydint->__value_f_call_counter           == 2 );
    BOOST_CHECK( mydint->__set_value_f_call_counter       == 1 );
    BOOST_CHECK( mydint->__on_get_callback_f_call_counter == 0 );
    BOOST_CHECK( mydint->__on_set_callback_f_call_counter == 1 );
    

    doocs_adapter->set(0);
    doocs_adapter->get  ();
    mydint       ->value();

    BOOST_CHECK( mydint->__value_f_call_counter           == 3 );
    BOOST_CHECK( mydint->__set_value_f_call_counter       == 2 );
    BOOST_CHECK( mydint->__on_get_callback_f_call_counter == 0 );
    BOOST_CHECK( mydint->__on_set_callback_f_call_counter == 2 );
    

    mydint       ->set_value(1);
    mydint       ->value();
    doocs_adapter->get  ();

    BOOST_CHECK( mydint->__value_f_call_counter           == 4 );
    BOOST_CHECK( mydint->__set_value_f_call_counter       == 3 );
    BOOST_CHECK( mydint->__on_get_callback_f_call_counter == 0 );
    BOOST_CHECK( mydint->__on_set_callback_f_call_counter == 2 );
    

    mydint       ->set_value(0);
    mydint       ->value();
    doocs_adapter->get  ();
	
    BOOST_CHECK( mydint->__value_f_call_counter           == 5 );
    BOOST_CHECK( mydint->__set_value_f_call_counter       == 4 );
    BOOST_CHECK( mydint->__on_get_callback_f_call_counter == 0 );
    BOOST_CHECK( mydint->__on_set_callback_f_call_counter == 2 );
    


	doocs_adapter->set(0);

    BOOST_CHECK( mydint->__value_f_call_counter           == 5 );
    BOOST_CHECK( mydint->__set_value_f_call_counter       == 5 );
    BOOST_CHECK( mydint->__on_get_callback_f_call_counter == 0 );
    BOOST_CHECK( mydint->__on_set_callback_f_call_counter == 3 );
    
}

BOOST_AUTO_TEST_CASE( test_sync1 )
{
    
    BOOST_CHECK( doocs_adapter->get  () == 0 );  // "reset"
    BOOST_CHECK( mydint       ->value() == 0 );


    doocs_adapter->set(1);
    BOOST_CHECK( doocs_adapter->get  () == 1 );
    BOOST_CHECK( mydint       ->value() == 1 );  // <---

    doocs_adapter->set(0);
    BOOST_CHECK( doocs_adapter->get  () == 0 );
    BOOST_CHECK( mydint       ->value() == 0 );  // <---

    mydint->set_value(1);
    BOOST_CHECK( mydint       ->value() == 1 );
    BOOST_CHECK( doocs_adapter->get  () == 1 );  // <---

    mydint->set_value(0);
    BOOST_CHECK( mydint       ->value() == 0 );
    BOOST_CHECK( doocs_adapter->get  () == 0 );  // <---
	

	doocs_adapter->set(0);
}

BOOST_AUTO_TEST_SUITE_END()

