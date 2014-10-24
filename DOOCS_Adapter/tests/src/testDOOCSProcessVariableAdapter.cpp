#define BOOST_TEST_MODULE callback_model_test
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "DOOCSProcessVariableAdapter.hpp"





struct TestStuffFixture {
	
	unsigned int _get_cb_counter;
	unsigned int _set_cb_counter;
	unsigned int _set_cb_counter_equals;
	
	int _cbTestVar;		// callback functionality test variable


	TestStuffFixture() : _get_cb_counter(0), _set_cb_counter(0), _cbTestVar(0) {}
	~TestStuffFixture() {}
	
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


	DOOCSPVAdapter doocs_adapter;
};




BOOST_FIXTURE_TEST_SUITE( test_callbacks, TestStuffFixture )

BOOST_AUTO_TEST_CASE( test_get )
{
	doocs_adapter.setOnGetCallbackFunction(boost::bind (&TestStuffFixture::on_get_callback, this));
	
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );

	_cbTestVar = 5;
	
    BOOST_CHECK( doocs_adapter.get() == 5 );
    BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _get_cb_counter == 1 );

    BOOST_CHECK( doocs_adapter.get() == 5 );
    BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _get_cb_counter == 2 );

	_cbTestVar = 10;
	
    BOOST_CHECK( doocs_adapter.get() == 10 );
    BOOST_CHECK( _cbTestVar      == 10 );
	BOOST_CHECK( _get_cb_counter == 3 );


	doocs_adapter.clearOnGetCallbackFunction();

    BOOST_CHECK( doocs_adapter.get() == 10 );
    BOOST_CHECK( _cbTestVar      == 10 );
	BOOST_CHECK( _get_cb_counter == 3 );
	
	
	reset_fixture();
}


BOOST_AUTO_TEST_CASE( test_set )
{
	doocs_adapter.setOnSetCallbackFunction(boost::bind (&TestStuffFixture::on_set_callback, this, _1, _2));
	
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );
	
	doocs_adapter.set(5);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 1 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter.set(5);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 2 );
	BOOST_CHECK( _set_cb_counter_equals == 1 );


	doocs_adapter.clearOnSetCallbackFunction();

	doocs_adapter.set(5);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 2 );
	BOOST_CHECK( _set_cb_counter_equals == 1 );

	doocs_adapter.set(15);

	BOOST_CHECK( _cbTestVar      == 5 );
	BOOST_CHECK( _set_cb_counter == 2 );
	BOOST_CHECK( _set_cb_counter_equals == 1 );

	
	reset_fixture();
}

BOOST_AUTO_TEST_SUITE_END()




BOOST_FIXTURE_TEST_SUITE( test_no_callbacks, TestStuffFixture )

BOOST_AUTO_TEST_CASE( test_setwithoutcallback_getwithoutcallback )
{
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter.setWithoutCallback(5);
	
    BOOST_CHECK( doocs_adapter.getWithoutCallback() == 5 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter.setWithoutCallback(10);
	
    BOOST_CHECK( doocs_adapter.getWithoutCallback() == 10 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	
	reset_fixture();
}


BOOST_AUTO_TEST_CASE( test_set_get__no_callbacks )
{
	BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter.set(5);
	
    BOOST_CHECK( doocs_adapter.get() == 5 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	doocs_adapter.set(10);
	
    BOOST_CHECK( doocs_adapter.get() == 10 );
    BOOST_CHECK( _cbTestVar      == 0 );
	BOOST_CHECK( _get_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter == 0 );
	BOOST_CHECK( _set_cb_counter_equals == 0 );

	
	reset_fixture();
}

BOOST_AUTO_TEST_SUITE_END()

