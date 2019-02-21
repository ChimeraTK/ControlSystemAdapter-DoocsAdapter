#ifndef __m4uD_array_testCases_numericals__
#define __m4uD_array_testCases_numericals__

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

// for: int, double, float

BOOST_FIXTURE_TEST_SUITE(
    test_operation,
    CallbacksTestFixture) // operation check: return values matter

BOOST_AUTO_TEST_CASE(test_length) { BOOST_CHECK_EQUAL(mydarray.length(), 4); }

BOOST_AUTO_TEST_CASE(test_getset_nocb) {
  // vec_test = [0,0,0,0]
  vec_test.assign(4, 0.0);

  const TypedVector &vec_mydarray = mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray.begin(), vec_mydarray.end(),
                                vec_test.begin(), vec_test.end());

  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);
  mydarray.setspectrum(vec_test, *pa);

  const TypedVector &vec_mydarray1 = mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray1.begin(), vec_mydarray1.end(),
                                vec_test.begin(), vec_test.end());
}

BOOST_AUTO_TEST_CASE(test_getwcsetwc_nocb) {
  // vec_test = [0,0,0,0]
  vec_test.assign(4, 0.0);

  const TypedVector &vec_mydarray = mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray.begin(), vec_mydarray.end(),
                                vec_test.begin(), vec_test.end());

  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);
  mydarray.setspectrum_without_callback(vec_test);

  const TypedVector &vec_mydarray1 = mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray1.begin(), vec_mydarray1.end(),
                                vec_test.begin(), vec_test.end());
}

BOOST_AUTO_TEST_CASE(test_randomaccess) // a "random access"
{
  mydarray.fill_spectrum(0, 1.0);
  mydarray.fill_spectrum(1, 2.0);
  mydarray.fill_spectrum(2, 3.0);
  mydarray.fill_spectrum(3, 4.0);

  BOOST_CHECK_EQUAL(mydarray.read_spectrum(0), 1.0);
  BOOST_CHECK_EQUAL(mydarray.read_spectrum(1), 2.0);
  BOOST_CHECK_EQUAL(mydarray.read_spectrum(2), 3.0);
  BOOST_CHECK_EQUAL(mydarray.read_spectrum(3), 4.0);
}

BOOST_AUTO_TEST_CASE(test_getset_cb) {
  // vec_test = [0,0,0,0]
  vec_test.assign(4, 0.0);

  const TypedVector &vec_mydarray = mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray.begin(), vec_mydarray.end(),
                                vec_test.begin(), vec_test.end());

  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);
  mydarray.setspectrum(vec_test, *pa);

  const TypedVector &vec_mydarray1 = mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray1.begin(), vec_mydarray1.end(),
                                vec_test.begin(), vec_test.end());

  mydarray.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1));

  // vec_test = [2,2,2,2], then [5,2,2,2]
  vec_test.assign(4, 2.0);
  mydarray.setspectrum(vec_test, *pa);
  vec_test[0] = 5; // vec_test[3] = 6;
  const TypedVector &vec_mydarray2 = mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray2.begin(), vec_mydarray2.end(),
                                vec_test.begin(), vec_test.end());

  mydarray.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this, _1));

  // vec_test = [3,3,3,3], then [5,3,3,6]
  vec_test.assign(4, 3.0);
  mydarray.setspectrum(vec_test, *pa);
  vec_test[0] = 5;
  vec_test[3] = 6;
  const TypedVector &vec_mydarray3 = mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray3.begin(), vec_mydarray3.end(),
                                vec_test.begin(), vec_test.end());
}

BOOST_AUTO_TEST_CASE(test_getwcsetwc_cb) {
  // vec_test = [0,0,0,0]
  vec_test.assign(4, 0.0);

  const TypedVector &vec_mydarray = mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray.begin(), vec_mydarray.end(),
                                vec_test.begin(), vec_test.end());

  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);
  mydarray.setspectrum_without_callback(vec_test);

  const TypedVector &vec_mydarray1 = mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray1.begin(), vec_mydarray1.end(),
                                vec_test.begin(), vec_test.end());

  mydarray.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1));

  // vec_test = [2,2,2,2]
  vec_test.assign(4, 2.0);
  mydarray.setspectrum_without_callback(vec_test);
  // vec_test[0] = 5;// vec_test[3] = 6;
  const TypedVector &vec_mydarray2 = mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray2.begin(), vec_mydarray2.end(),
                                vec_test.begin(), vec_test.end());

  mydarray.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this, _1));

  // vec_test = [3,3,3,3]
  vec_test.assign(4, 3.0);
  mydarray.setspectrum_without_callback(vec_test);
  // vec_test[0] = 5; vec_test[3] = 6;
  const TypedVector &vec_mydarray3 = mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL_COLLECTIONS(vec_mydarray3.begin(), vec_mydarray3.end(),
                                vec_test.begin(), vec_test.end());
}

BOOST_AUTO_TEST_CASE(test_cachesync) {
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);

  mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(),
                    true); // D_spectrum has 0's after creation

  // vec_test = [0,0,0,0]
  vec_test.assign(4, 0.0);

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);
  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), false);

  mydarray.getspectrum(*pa);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  // vec_test = [0,0,0,0]
  vec_test.assign(4, 0.0);

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), false);

  mydarray.getspectrum_without_callback();
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  mydarray.fill_spectrum(0, 0.0);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);
  mydarray.read_spectrum(0);
  mydarray.read_spectrum(1);
  mydarray.read_spectrum(2);
  mydarray.read_spectrum(3);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  mydarray.fill_spectrum(1, 2.0);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), false);
  mydarray.read_spectrum(0);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), false);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), false);

  TypedVector v(4);
  mydarray.fillVector(v);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
  BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);

  // FIXME: a series for callbacks presence advised?
  /*
// vec_test = [0,0,0,0]
vec_test.assign(4, 0.0);

vec_mydarray = mydarray.getspectrum(*pa);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);


// vec_test = [1,1,1,1]
vec_test.assign(4, 1.0);
mydarray.setspectrum(vec_test, *pa);

vec_mydarray = mydarray.getspectrum(*pa);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);


  mydarray.setOnSetCallbackFunction(boost::bind
(&CallbacksTestFixture::on_set_callback, this, _1));

// vec_test = [2,2,2,2], then [5,2,2,2]
vec_test.assign(4, 2.0);
mydarray.setspectrum(vec_test, *pa);
                                                                   vec_test[0] =
5;// vec_test[3] = 6; vec_mydarray = mydarray.getspectrum(*pa);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);


  mydarray.setOnGetCallbackFunction(boost::bind
(&CallbacksTestFixture::on_get_callback, this, _1));

// vec_test = [3,3,3,3], then [5,3,3,6]
vec_test.assign(4, 3.0);
mydarray.setspectrum(vec_test, *pa);
                                                                   vec_test[0] =
5; vec_test[3] = 6; vec_mydarray = mydarray.getspectrum(*pa);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);




// vec_test = [0,0,0,0]
vec_test.assign(4, 0.0);

vec_mydarray = mydarray.getspectrum_without_callback();
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);


// vec_test = [1,1,1,1]
vec_test.assign(4, 1.0);
mydarray.setspectrum_without_callback(vec_test);

vec_mydarray = mydarray.getspectrum_without_callback();
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);


  mydarray.setOnSetCallbackFunction(boost::bind
(&CallbacksTestFixture::on_set_callback, this, _1));

// vec_test = [2,2,2,2]
vec_test.assign(4, 2.0);
mydarray.setspectrum_without_callback(vec_test);
                                                                  // vec_test[0]
= 5;// vec_test[3] = 6; vec_mydarray = mydarray.getspectrum_without_callback();
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);


  mydarray.setOnGetCallbackFunction(boost::bind
(&CallbacksTestFixture::on_get_callback, this, _1));

// vec_test = [3,3,3,3]
vec_test.assign(4, 3.0);
mydarray.setspectrum_without_callback(vec_test);
                                                                  // vec_test[0]
= 5; vec_test[3] = 6; vec_mydarray = mydarray.getspectrum_without_callback();
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_flag(), true);
BOOST_CHECK_EQUAL(mydarray.__is_cache_synced_vect(), true);
  */
}

BOOST_AUTO_TEST_CASE(test_fill) {
  mydarray.fill(30);

  for (int i = 0; i < mydarray.length(); ++i)
    BOOST_CHECK_EQUAL(mydarray.read_spectrum(i), 30);
}

BOOST_AUTO_TEST_CASE(test_fillvector) {
  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);
  mydarray.setspectrum(vec_test, *pa);

  TypedVector v(4);
  mydarray.fillVector(v);

  BOOST_CHECK_EQUAL_COLLECTIONS(v.begin(), v.end(), vec_test.begin(),
                                vec_test.end());
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================

BOOST_FIXTURE_TEST_SUITE(
    test_callbacks,
    CallbacksTestFixture) // callback arbitrarily present: callback counters
                          // matter, return values don't

BOOST_AUTO_TEST_CASE(test_get_cb_count) {
  BOOST_CHECK(_get_cb_counter == 0);

  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 0);

  mydarray.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this, _1));

  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 1);
  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 2);

  mydarray.clearOnGetCallbackFunction();

  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 2);
  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 2);

  mydarray.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this, _1));

  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 3);
  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 4);

  mydarray.clearOnGetCallbackFunction();

  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 4);
  mydarray.getspectrum(*pa);
  BOOST_CHECK(_get_cb_counter == 4);
}

BOOST_AUTO_TEST_CASE(test_set_cb_count) {
  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);

  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1));

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 1);

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 2);

  mydarray.clearOnSetCallbackFunction();

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 2);

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 2);

  mydarray.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1));

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 3);

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 4);

  mydarray.clearOnSetCallbackFunction();

  mydarray.setspectrum(vec_test, *pa);
  BOOST_CHECK(_set_cb_counter == 4);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================

BOOST_FIXTURE_TEST_SUITE(
    test_no_callbacks,
    CallbacksTestFixture) // callback absent: callback counters matter, return
                          // values don't

BOOST_AUTO_TEST_CASE(test_get_nocb_count) {
  BOOST_CHECK(_get_cb_counter == 0);

  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydarray.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this, _1));

  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);
  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydarray.clearOnGetCallbackFunction();

  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);
  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydarray.setOnGetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_get_callback, this, _1));

  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);
  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);

  mydarray.clearOnGetCallbackFunction();

  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);
  mydarray.getspectrum_without_callback();
  BOOST_CHECK(_get_cb_counter == 0);
}

BOOST_AUTO_TEST_CASE(test_set_nocb_count) {
  // vec_test = [1,1,1,1]
  vec_test.assign(4, 1.0);

  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1));

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.clearOnSetCallbackFunction();

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setOnSetCallbackFunction(
      boost::bind(&CallbacksTestFixture::on_set_callback, this, _1));

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);

  mydarray.clearOnSetCallbackFunction();

  mydarray.setspectrum_without_callback(vec_test);
  BOOST_CHECK(_set_cb_counter == 0);
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* __m4uD_array_testCases_numericals__ */
