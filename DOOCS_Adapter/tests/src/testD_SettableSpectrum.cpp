#define BOOST_TEST_MODULE test_dsettablespectrum
#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_tools.hpp>
using namespace boost::unit_test;


#include "D_SettableSpectrum.hpp"

#include <vector>






// ============================================================================


struct D_SettableSpectrumTestFixture {
    
    D_SettableSpectrum sspec;

    D_SettableSpectrumTestFixture() : sspec(NULL, 4, NULL) {}

};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


BOOST_FIXTURE_TEST_CASE( get, D_SettableSpectrumTestFixture )
{
    std::vector<float> expected;
    expected.push_back(1);
    expected.push_back(2);
    expected.push_back(3);
    expected.push_back(4);

    std::vector<float> actual(4,0);
    
    sspec.fill_spectrum(0, 1);
	sspec.fill_spectrum(1, 2);
	sspec.fill_spectrum(2, 3);
	sspec.fill_spectrum(3, 4);
    
    sspec.get_spectrum(actual);

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), actual.begin(), actual.end());
}

BOOST_FIXTURE_TEST_CASE( set, D_SettableSpectrumTestFixture )
{
    std::vector<float> expected;
    expected.push_back(1);
    expected.push_back(2);
    expected.push_back(3);
    expected.push_back(4);

    std::vector<float> actual(4,0);
    
    sspec.set_spectrum(expected);
    
    actual[0] = sspec.read_spectrum(0);
    actual[1] = sspec.read_spectrum(1);
    actual[2] = sspec.read_spectrum(2);
    actual[3] = sspec.read_spectrum(3);
    
    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), actual.begin(), actual.end());
}

BOOST_FIXTURE_TEST_CASE( fill, D_SettableSpectrumTestFixture )
{
    std::vector<float> expected;
    expected.push_back(1);
    expected.push_back(1);
    expected.push_back(1);
    expected.push_back(1);

    std::vector<float> actual(4,0);
    
    sspec.fill(1);
    
    actual[0] = sspec.read_spectrum(1);
    actual[1] = sspec.read_spectrum(1);
    actual[2] = sspec.read_spectrum(1);
    actual[3] = sspec.read_spectrum(1);
    
    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), actual.begin(), actual.end());
}
