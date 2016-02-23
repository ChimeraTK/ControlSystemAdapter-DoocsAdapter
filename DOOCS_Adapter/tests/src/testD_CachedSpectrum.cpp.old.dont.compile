#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_tools.hpp>
using namespace boost::unit_test_framework;


#include <vector>

#include "eq_fct.h"
#include "D_CachedSpectrum.hpp"




namespace mtca4u {


////////////////////////////////////////////////////////////////---<>
                                                              //
template <typename T>
class D_CachedSpectrumTest
{

private:

    D_CachedSpectrum<T> cspec;


public:

    D_CachedSpectrumTest() : cspec(NULL, 4, NULL) {}
    
    void test_cachesync();
    void test_fillvector();
    void test_get_spectrum();

};
                                                              //
////////////////////////////////////////////////////////////////---<>


template <typename T>
void D_CachedSpectrumTest<T>::test_cachesync()
{
    std::vector<T> vec_get(4, 0.0);
    std::vector<T> vec_test;

                                    // spectrum=[0,0,0,0] cache=[0,0,0,0]
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_flag(), false);

    cspec.fill_spectrum(0, 5);      // spectrum=[5,0,0,0] cache=[0,0,0,0]
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_flag(), false);
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_vect(), false);		

    const std::vector<T> & vec_getr = 
    cspec.get_spectrum();           // spectrum=[5,0,0,0] cache=[5,0,0,0]
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_flag(), true);
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_vect(), true);


    vec_test.assign(4, 1.0);

    cspec.set_spectrum(vec_test);   // spectrum=[1,1,1,1] cache=[5,0,0,0]
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_flag(), false);
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_vect(), false);

    cspec.fillVector(vec_get);      // spectrum=[1,1,1,1] cache=[1,1,1,1]
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_flag(), true);
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_vect(), true);

    cspec.fill(5);                  // spectrum=[5,5,5,5] cache=[1,1,1,1]
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_flag(), false);
        BOOST_CHECK_EQUAL(cspec.__is_cache_synced_vect(), false);
        
        
    (void)vec_getr; // suppress unused variable
}


template <typename T>
void D_CachedSpectrumTest<T>::test_fillvector()
{
    std::vector<T> vec_test1(4, 1.0);
    cspec.set_spectrum(vec_test1);   // spectrum=[1,1,1,1]

    std::vector<T> vec_get(4);
    cspec.fillVector(vec_get);

        BOOST_CHECK_EQUAL_COLLECTIONS(vec_get.begin(), vec_get.end(), vec_test1.begin(), vec_test1.end());


    std::vector<T> vec_test2(4, 2.0);
    cspec.set_spectrum(vec_test2);   // spectrum=[2,2,2,2]

        BOOST_CHECK_EQUAL_COLLECTIONS(vec_get.begin(), vec_get.end(), vec_test1.begin(), vec_test1.end()); // still
}


template <typename T>
void D_CachedSpectrumTest<T>::test_get_spectrum()
{
    std::vector<T> vec_test1(4, 1.0);
    cspec.set_spectrum(vec_test1);   // spectrum=[1,1,1,1]

    const std::vector<T> & vec_getr = 
    cspec.get_spectrum();

        BOOST_CHECK_EQUAL_COLLECTIONS(vec_getr.begin(), vec_getr.end(), vec_test1.begin(), vec_test1.end());
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>
                                                                                                                                              //
/** The boost test suite.
 */
template <typename T>
class D_CachedSpectrumTestSuite : public test_suite
{

public:

    D_CachedSpectrumTestSuite() : test_suite("D_CachedSpectrum test suite")
    {
        boost::shared_ptr< D_CachedSpectrumTest<T> > 	dcachedSpectrumTest( new D_CachedSpectrumTest<T> );
        
        add( BOOST_CLASS_TEST_CASE( &D_CachedSpectrumTest<T>::test_cachesync,  dcachedSpectrumTest ) );
        add( BOOST_CLASS_TEST_CASE( &D_CachedSpectrumTest<T>::test_fillvector, dcachedSpectrumTest ) );
    }
};
                                                                                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>

} //namespace mtca4u

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>
                                                                                                                                              //
test_suite * init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
    framework::master_test_suite().p_name.value = "D_CachedSpectrum test suite";
    
    framework::master_test_suite().add( new mtca4u::D_CachedSpectrumTestSuite<int> );           // FIXME (namespace for classes)
    framework::master_test_suite().add( new mtca4u::D_CachedSpectrumTestSuite<double>);
    framework::master_test_suite().add( new mtca4u::D_CachedSpectrumTestSuite<float>);
    
    return NULL;
}
                                                                                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////---<>

