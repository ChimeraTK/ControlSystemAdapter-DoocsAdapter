#define BOOST_TEST_MODULE test_spectrum_mock
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "d_fct.h"






// ============================================================================


struct D_spectrumTestFixture {
    
    D_spectrum dspec;

    D_spectrumTestFixture() : dspec(NULL, 4, NULL) {}

};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


BOOST_FIXTURE_TEST_CASE( length, D_spectrumTestFixture )
{
    BOOST_CHECK_EQUAL( dspec.length(), 4 );
}

BOOST_FIXTURE_TEST_CASE( fill_read, D_spectrumTestFixture )
{
    BOOST_CHECK_EQUAL( dspec.read_spectrum(0), 0 );
    BOOST_CHECK_EQUAL( dspec.read_spectrum(1), 0 );
    BOOST_CHECK_EQUAL( dspec.read_spectrum(2), 0 );
    BOOST_CHECK_EQUAL( dspec.read_spectrum(3), 0 );

    dspec.fill_spectrum(0, 1);
	dspec.fill_spectrum(1, 2);
	dspec.fill_spectrum(2, 3);
	dspec.fill_spectrum(3, 4);

    BOOST_CHECK_EQUAL( dspec.read_spectrum(0), 1 );
    BOOST_CHECK_EQUAL( dspec.read_spectrum(1), 2 );
    BOOST_CHECK_EQUAL( dspec.read_spectrum(2), 3 );
    BOOST_CHECK_EQUAL( dspec.read_spectrum(3), 4 );
}

BOOST_FIXTURE_TEST_CASE( fill_read_outofrange, D_spectrumTestFixture )
{
    dspec.fill_spectrum(-1, 1);
	dspec.fill_spectrum(4, 2);

    BOOST_CHECK_EQUAL( dspec.read_spectrum(-1), 0 );
    BOOST_CHECK_EQUAL( dspec.read_spectrum(4), 0 );
}
