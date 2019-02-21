#ifndef __m4uD_type_testCases_numericals__
#define __m4uD_type_testCases_numericals__

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

// for: int, double, float

BOOST_FIXTURE_TEST_SUITE(
    test_operation,
    CallbacksTestFixture) // operation check: return values matter

BOOST_AUTO_TEST_CASE(test_getset_nocb) {
  BOOST_CHECK(mydtype.getval() == 0);

  mydtype.setval(1);
  BOOST_CHECK(mydtype.getval() == 1); // <---

  mydtype.setval(0);
  BOOST_CHECK(mydtype.getval() == 0); // <---

  mydtype.setval(3);
  BOOST_CHECK(mydtype.getval() == 3); // <---

  mydtype.setval(0);
  BOOST_CHECK(mydtype.getval() == 0); // <---
}

BOOST_AUTO_TEST_CASE(test_getwcsetwc_nocb) {
  BOOST_CHECK(mydtype.getval_without_callback() == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(mydtype.getval_without_callback() == 1); // <---

  mydtype.setval_without_callback(0);
  BOOST_CHECK(mydtype.getval_without_callback() == 0); // <---

  mydtype.setval_without_callback(3);
  BOOST_CHECK(mydtype.getval_without_callback() == 3); // <---

  mydtype.setval_without_callback(0);
  BOOST_CHECK(mydtype.getval_without_callback() == 0); // <---
}

BOOST_AUTO_TEST_CASE(test_getset_cb) {
  BOOST_CHECK(mydtype.getval() == 0);

  mydtype.setval(1);
  BOOST_CHECK(mydtype.getval() == 1); // <---

  mydtype.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1, _2));
  mydtype.setval(2);
  BOOST_CHECK(mydtype.getval() == 2); // <---

  mydtype.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this));
  mydtype.setval(3);
  BOOST_CHECK(mydtype.getval() == 0); // <---

  mydtype.setval(4);
  BOOST_CHECK(mydtype.getval() == 0); // <---
}

BOOST_AUTO_TEST_CASE(test_getwcsetwc_cb) {
  BOOST_CHECK(mydtype.getval_without_callback() == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(mydtype.getval_without_callback() == 1); // <---

  mydtype.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1, _2));
  mydtype.setval_without_callback(2);
  BOOST_CHECK(mydtype.getval_without_callback() == 2); // <---

  mydtype.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this));
  mydtype.setval_without_callback(3);
  BOOST_CHECK(mydtype.getval_without_callback() == 3); // <---

  mydtype.setval_without_callback(4);
  BOOST_CHECK(mydtype.getval_without_callback() == 4); // <---
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================

BOOST_FIXTURE_TEST_SUITE(
    test_callbacks,
    CallbacksTestFixture) // callback arbitrarily present: callback counters
                          // matter, return values don't

BOOST_AUTO_TEST_CASE(test_get_cb_count) {
  BOOST_CHECK(_get_cb_counter == 0);

  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 0);

  mydtype.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this));

  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 1);
  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 2);

  mydtype.clearOnGetCallbackFunction();

  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 2);
  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 2);

  mydtype.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this));

  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 3);
  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 4);

  mydtype.clearOnGetCallbackFunction();

  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 4);
  mydtype.getval();
  BOOST_CHECK(_get_cb_counter == 4);
}

BOOST_AUTO_TEST_CASE(test_set_cb_count) {
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1, _2));

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 1);
  BOOST_CHECK(_set_cb_counter_equals == 1);

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 2);
  BOOST_CHECK(_set_cb_counter_equals == 2);

  mydtype.clearOnSetCallbackFunction();

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 2);
  BOOST_CHECK(_set_cb_counter_equals == 2);

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 2);
  BOOST_CHECK(_set_cb_counter_equals == 2);

  mydtype.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1, _2));

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 3);
  BOOST_CHECK(_set_cb_counter_equals == 3);

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 4);
  BOOST_CHECK(_set_cb_counter_equals == 4);

  mydtype.clearOnSetCallbackFunction();

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 4);
  BOOST_CHECK(_set_cb_counter_equals == 4);
}

BOOST_AUTO_TEST_CASE(test_set_cb_equals_count) {
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1, _2));

  mydtype.setval(1);
  BOOST_CHECK(_set_cb_counter == 1);

  mydtype.setval(2);
  BOOST_CHECK(_set_cb_counter == 2);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval(2);
  BOOST_CHECK(_set_cb_counter == 3);
  BOOST_CHECK(_set_cb_counter_equals == 1);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================

BOOST_FIXTURE_TEST_SUITE(
    test_no_callbacks,
    CallbacksTestFixture) // callback absent: callback counters matter, return
                          // values don't

BOOST_AUTO_TEST_CASE(test_get_nocb_count) {
  BOOST_CHECK(_get_cb_counter == 0);

  mydtype.getval_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydtype.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this));

  mydtype.getval_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydtype.clearOnGetCallbackFunction();

  mydtype.getval_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydtype.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this));

  mydtype.getval_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydtype.clearOnGetCallbackFunction();

  mydtype.getval_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);
}

BOOST_AUTO_TEST_CASE(test_set_nocb_count) {
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1, _2));

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.clearOnSetCallbackFunction();

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1, _2));

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.clearOnSetCallbackFunction();

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);

  mydtype.setval_without_callback(1);
  BOOST_CHECK(_set_cb_counter == 0);
  BOOST_CHECK(_set_cb_counter_equals == 0);
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* __m4uD_type_testCases_numericals__ */
