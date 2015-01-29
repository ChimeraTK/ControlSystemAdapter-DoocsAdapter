#define BOOST_TEST_MODULE test_string_mock
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;


#include "d_fct.h"






// ============================================================================


struct D_stringTestFixture {
    
    std::string tests1;
    std::string tests2;
    std::string testsl;

    D_string * dstr;


    D_stringTestFixture()
    {
        tests1 = "test string 1";
        tests2 = "test string 2";
        testsl = "test string looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong";  // >80
        
        dstr = new D_string(NULL, NULL);
    }

    ~D_stringTestFixture()
    {
        delete dstr;
    }
    
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


BOOST_FIXTURE_TEST_CASE( set1, D_stringTestFixture )
{
    dstr->set_value(tests1.c_str());
    BOOST_CHECK_EQUAL( dstr->value(), tests1.c_str() );
}

BOOST_FIXTURE_TEST_CASE( set2, D_stringTestFixture )
{
    const char * str_loc = "content 1";
    dstr->set_value(str_loc);
    BOOST_CHECK_EQUAL( dstr->value(), str_loc );
}

BOOST_FIXTURE_TEST_CASE( set3, D_stringTestFixture )
{
    const char * str_loc   = "first value";
    const char * first_val = "first value";
    dstr->set_value(str_loc);
                 str_loc = "new content";
    BOOST_CHECK_EQUAL( dstr->value(), first_val );
}

BOOST_FIXTURE_TEST_CASE( set4, D_stringTestFixture )
{
    const char * str_loc = "content 1";
    dstr->set_value(str_loc);
                 str_loc = "content different than 1";
    BOOST_CHECK_EQUAL( dstr->value(), "content 1" );
}

BOOST_FIXTURE_TEST_CASE( set5, D_stringTestFixture ) // DOOCS STRING_LENGTH length limitation not working for dynamically allocated strings. Nice.
{
    dstr->set_value(testsl.c_str());
    BOOST_CHECK_EQUAL( dstr->value(), testsl.c_str() );
}

BOOST_FIXTURE_TEST_CASE( set6, D_stringTestFixture )
{
    const char * str_loc = "content 1";
    dstr->set_value("content 1");
    BOOST_CHECK_EQUAL( dstr->value(), str_loc );
}

