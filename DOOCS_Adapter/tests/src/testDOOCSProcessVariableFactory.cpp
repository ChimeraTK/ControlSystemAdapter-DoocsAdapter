#define BOOST_TEST_MODULE test_dpva_f


#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

#include "DOOCSProcessVariableFactory.hpp"





BOOST_AUTO_TEST_CASE( testGetProcessVariable )
{
    mtca4u::DOOCSProcessVariableFactory DOOCSPVFactory( NULL );


    boost::shared_ptr<mtca4u::ProcessVariable<int> > firstInteger  = DOOCSPVFactory.getProcessVariable<int>("firstInteger");

    firstInteger->set(42);

    boost::shared_ptr<mtca4u::ProcessVariable<int> > secondInteger = DOOCSPVFactory.getProcessVariable<int>("firstInteger");
    boost::shared_ptr<mtca4u::ProcessVariable<int> > thirdInteger  = DOOCSPVFactory.getProcessVariable<int>("thirdInteger");

    thirdInteger->set(55);

    // make sure the second integer is the same instance as the first integer, but the third is not
    BOOST_CHECK_EQUAL(*secondInteger, 42);
    secondInteger->set(43);
    BOOST_CHECK_EQUAL(*firstInteger, 43);
    BOOST_CHECK_EQUAL(*thirdInteger, 55);

    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessVariable<uint64_t>("tooLong"), std::bad_typeid );
    BOOST_CHECK_THROW( DOOCSPVFactory.getProcessVariable<double>("firstInteger"), std::bad_cast );  // FIXME why not boost::bad_any_cast?

    // check that name conflicts with arrays work
    //~ boost::shared_ptr<mtca4u::ProcessArray<int> > intArray = DOOCSPVFactory.getProcessArray<int>("intArray", 42);
    //~ BOOST_CHECK_THROW( DOOCSPVFactory.getProcessVariable<int>("intArray"),  boost::bad_any_cast );
}

/*
BOOST_AUTO_TEST_CASE( testGetProcessVariable )
{
    mtca4u::StubProcessVariableFactory stubProcessVariableFactory;

    boost::shared_ptr<mtca4u::ProcessVariable<int> > firstInteger  = stubProcessVariableFactory.getProcessVariable<int>("firstInteger");

    firstInteger->set(42);

    boost::shared_ptr<mtca4u::ProcessVariable<int> > secondInteger = stubProcessVariableFactory.getProcessVariable<int>("firstInteger");
    boost::shared_ptr<mtca4u::ProcessVariable<int> > thirdInteger  = stubProcessVariableFactory.getProcessVariable<int>("thirdInteger");

    thirdInteger->set(55);

    // make sure the second integer is the same instance as the first integer, but the third is not
    BOOST_CHECK(*secondInteger == 42);
    secondInteger->set(43);
    BOOST_CHECK(*firstInteger == 43);
    BOOST_CHECK(*thirdInteger == 55);

    BOOST_CHECK_THROW( stubProcessVariableFactory.getProcessVariable<uint64_t>("tooLong"), std::bad_typeid );
    BOOST_CHECK_THROW( stubProcessVariableFactory.getProcessVariable<double>("firstInteger"), boost::bad_any_cast );

    // check that name conflicts with arrays work
    boost::shared_ptr<mtca4u::ProcessArray<int> > intArray = stubProcessVariableFactory.getProcessArray<int>("intArray", 42);
    BOOST_CHECK_THROW( stubProcessVariableFactory.getProcessVariable<int>("intArray"),  boost::bad_any_cast );
}
*/
