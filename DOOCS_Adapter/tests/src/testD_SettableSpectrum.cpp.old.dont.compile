#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_tools.hpp>
using namespace boost::unit_test_framework;


#include <vector>

#include "eq_fct.h"
#include "D_SettableSpectrum.hpp"




namespace mtca4u {


////////////////////////////////////////////////////////////////---<>
                                                              //
template <typename T>
class D_SettableSpectrumTest
{

private:

    D_SettableSpectrum<T> sspec;


public:

    D_SettableSpectrumTest() : sspec(NULL, 4, NULL) {}
    
    void testfill_spectrum();
    void testGet();
    void testSet();
    void testFill();

};
                                                              //
////////////////////////////////////////////////////////////////---<>



template <typename T>
void D_SettableSpectrumTest<T>::testfill_spectrum()
{
    std::vector<T> expected;
    expected.push_back(1);
    expected.push_back(2);
    expected.push_back(3);
    expected.push_back(4);

    sspec.fill_spectrum(0, expected[0]);
	sspec.fill_spectrum(1, expected[1]);
	sspec.fill_spectrum(2, expected[2]);
	sspec.fill_spectrum(3, expected[3]);
    
    float * buffer = sspec.spectrum()->d_spect_array.d_spect_array_val;

    std::vector<T> actual;
    
    actual.push_back( static_cast<T>(buffer[0]) );
    actual.push_back( static_cast<T>(buffer[1]) );
    actual.push_back( static_cast<T>(buffer[2]) );
    actual.push_back( static_cast<T>(buffer[3]) );

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), actual.begin(), actual.end());
}


template <typename T>
void D_SettableSpectrumTest<T>::testGet()
{
    std::vector<T> expected;
    expected.push_back(1);
    expected.push_back(2);
    expected.push_back(3);
    expected.push_back(4);

    std::vector<T> actual(4,0);
    
    sspec.fill_spectrum(0, 1);
	sspec.fill_spectrum(1, 2);
	sspec.fill_spectrum(2, 3);
	sspec.fill_spectrum(3, 4);
    
    sspec.get_spectrum_copy(actual);

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), actual.begin(), actual.end());
}


template <typename T>
void D_SettableSpectrumTest<T>::testSet()
{
    std::vector<T> expected;
    expected.push_back(1);
    expected.push_back(2);
    expected.push_back(3);
    expected.push_back(4);

    std::vector<T> actual(4,0);
    
    sspec.set_spectrum(expected);
    
    actual[0] = sspec.read_spectrum(0);
    actual[1] = sspec.read_spectrum(1);
    actual[2] = sspec.read_spectrum(2);
    actual[3] = sspec.read_spectrum(3);
    
    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), actual.begin(), actual.end());
}


template <typename T>
void D_SettableSpectrumTest<T>::testFill()
{
    std::vector<T> expected;
    expected.push_back(1);
    expected.push_back(1);
    expected.push_back(1);
    expected.push_back(1);

    std::vector<T> actual(4,0);
    
    sspec.fill(1);
    
    actual[0] = sspec.read_spectrum(1);
    actual[1] = sspec.read_spectrum(1);
    actual[2] = sspec.read_spectrum(1);
    actual[3] = sspec.read_spectrum(1);
    
    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), actual.begin(), actual.end());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>
                                                                                                                              //
/** The boost test suite.
 */
template <typename T>
class D_SettableSpectrumTestSuite : public test_suite
{

public:

    D_SettableSpectrumTestSuite() : test_suite("D_SettableSpectrum test suite")
    {
        boost::shared_ptr< D_SettableSpectrumTest<T> > 	dsettableSpectrumTest( new D_SettableSpectrumTest<T> );
        
        add( BOOST_CLASS_TEST_CASE( &D_SettableSpectrumTest<T>::testfill_spectrum,  dsettableSpectrumTest ) );
        add( BOOST_CLASS_TEST_CASE( &D_SettableSpectrumTest<T>::testSet,            dsettableSpectrumTest ) );
        add( BOOST_CLASS_TEST_CASE( &D_SettableSpectrumTest<T>::testGet,            dsettableSpectrumTest ) );
        add( BOOST_CLASS_TEST_CASE( &D_SettableSpectrumTest<T>::testFill,           dsettableSpectrumTest ) );
    }
};
                                                                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>

} //namespace mtca4u

/////////////////////////////////////////////////////////////////////////////////////////////////////////---<>
                                                                                                       //
test_suite * init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
    framework::master_test_suite().p_name.value = "D_SettableSpectrum test suite";
    
    framework::master_test_suite().add( new mtca4u::D_SettableSpectrumTestSuite<int> );
    framework::master_test_suite().add( new mtca4u::D_SettableSpectrumTestSuite<double>);
    framework::master_test_suite().add( new mtca4u::D_SettableSpectrumTestSuite<float>);
    
    return NULL;
}
                                                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////---<>

