#ifndef SERVER_BASED_TEST_TOOLS_H
#define SERVER_BASED_TEST_TOOLS_H

#include "basenameFromAddress.h"
#include <eq_fct.h>

// these constants don't exist in DOOCS, although they should. We define them
// here for better code readability
static const int ACCESS_RO = 0; // read only
static const int ACCESS_RW = 1; // read/write

#define DOOCS_ADAPTER_DEFAULT_FIXTURE                                                                                  \
  struct GlobalFixture {                                                                                               \
    GlobalFixture() { ChimeraTK::DoocsAdapter::waitUntilInitialised(); }                                               \
                                                                                                                       \
    ReferenceTestApplication referenceTestApplication{framework::master_test_suite().p_name.value};                    \
    ThreadedDoocsServer server{framework::master_test_suite().p_name.value + ".conf",                                  \
        framework::master_test_suite().argc, framework::master_test_suite().argv};                                     \
  };                                                                                                                   \
                                                                                                                       \
  BOOST_GLOBAL_FIXTURE(GlobalFixture);

#define DOOCS_ADAPTER_DEFAULT_FIXTURE_STATIC_APPLICATION                                                               \
  struct GlobalFixture {                                                                                               \
    GlobalFixture() {                                                                                                  \
      ChimeraTK::DoocsAdapter::waitUntilInitialised();                                                                 \
      GlobalFixture::referenceTestApplication.initialiseManualLoopControl();                                           \
    }                                                                                                                  \
                                                                                                                       \
    ~GlobalFixture() { GlobalFixture::referenceTestApplication.releaseManualLoopControl(); }                           \
                                                                                                                       \
    static ReferenceTestApplication referenceTestApplication;                                                          \
    ThreadedDoocsServer server{framework::master_test_suite().p_name.value + ".conf",                                  \
        framework::master_test_suite().argc, framework::master_test_suite().argv};                                     \
  };                                                                                                                   \
                                                                                                                       \
  ReferenceTestApplication GlobalFixture::referenceTestApplication{BOOST_STRINGIZE(BOOST_TEST_MODULE)};                \
  BOOST_GLOBAL_FIXTURE(GlobalFixture);

template<class DOOCS_T>
void checkHistory(DOOCS_T* property, bool expected_has_history) {
  bool has_history = property->get_histPointer();
  BOOST_CHECK_MESSAGE(has_history == expected_has_history,
      "History on/off wrong for " + property->basename() + ". Should be " + (expected_has_history ? "true" : "false"));
}

template<>
void checkHistory(D_spectrum* /*property*/, bool) {
  /// @todo FIXME implement check on D_spectrum history/ ring buffer
}
// there are no histories for arrays. Do nothing
template<>
void checkHistory(D_bytearray* /*property*/, bool) {}
template<>
void checkHistory(D_shortarray* /*property*/, bool) {}
template<>
void checkHistory(D_intarray* /*property*/, bool) {}
template<>
void checkHistory(D_longarray* /*property*/, bool) {}
template<>
void checkHistory(D_floatarray* /*property*/, bool) {}
template<>
void checkHistory(D_doublearray* /*property*/, bool) {}

EqFct* getLocationFromPropertyAddress(std::string const& propertyAddress) {
  EqAdr ad;
  // obtain location pointer
  ad.adr(propertyAddress.c_str());
  EqFct* eqFct = eq_get(&ad);
  BOOST_REQUIRE_MESSAGE(eqFct, "Could not get location for property " + propertyAddress);

  return eqFct;
}

/// Used internally.
/// ATTENTION: DOES NOT LOCK THE LOCATION. DO THIS MANUALLY BEFORE GETTING THE
/// PROPERTY!
template<class DOOCS_T>
DOOCS_T* getDoocsProperty(std::string const& propertyAddress) {
  auto eqFct = getLocationFromPropertyAddress(propertyAddress);
  auto propertyName = ChimeraTK::basenameFromAddress(propertyAddress);
  auto rawProperty = eqFct->find_property(propertyName);

  BOOST_REQUIRE_MESSAGE(rawProperty, "Could not find property address" << propertyAddress);
  auto property = dynamic_cast<DOOCS_T*>(rawProperty);

  BOOST_REQUIRE_MESSAGE(property,
      "Property " << propertyAddress << " has unexpected type: " << rawProperty->data_type_string() << ":"
                  << rawProperty->data_type());

  return property;
}

template<class DOOCS_T>
void checkDoocsProperty(std::string const& propertyAddress,
    bool expected_has_history = true,
    bool expected_is_writeable = true) {
  auto location = getLocationFromPropertyAddress(propertyAddress);
  location->lock();

  auto property = getDoocsProperty<DOOCS_T>(propertyAddress);

  checkHistory(property, expected_has_history);

  std::stringstream errorMessage;
  errorMessage << "Access rights not correct for '" << propertyAddress << "': access word is " << property->get_access()
               << ", expected " << (expected_is_writeable ? ACCESS_RW : ACCESS_RO);
  if(expected_is_writeable) {
    BOOST_CHECK_MESSAGE(property->get_access() == ACCESS_RW, errorMessage.str());
  }
  else {
    BOOST_CHECK(property->get_access() == ACCESS_RO);
  }

  location->unlock();
}

// this function does the locking so it can be used in a loop and frees the lock
// in between.
float readSpectrumStart(std::string const& propertyAddress) {
  auto location = getLocationFromPropertyAddress(propertyAddress);
  location->lock();

  auto spectrum = getDoocsProperty<D_spectrum>(propertyAddress);

  float start = spectrum->spec_start();
  location->unlock();

  return start;
}

// sorry for copying the code. It really sucks to wrap 10 lines of bloat around
// 1 line of content.
float readSpectrumIncrement(std::string const& propertyAddress) {
  auto location = getLocationFromPropertyAddress(propertyAddress);
  location->lock();

  auto spectrum = getDoocsProperty<D_spectrum>(propertyAddress);

  float increment = spectrum->spec_inc();
  location->unlock();

  return increment;
}

// BOOST_REQUIRE_MESSAGE(spectrum, "Could not find spectrum "<< propertyAddress
// <<", property does not exist or is not a spectrum.");
//
// EqFct *eqFct = eq_get(&ad);
// ASSERT(eqFct != NULL, std::string("Could not get location for property
// ")+name);
// // set spectrum
// eqFct->lock();
// p->set(&ad,&ed,&res);
// p->unlock();
//
//
//

void checkDataType(std::string const& propertyAddress, int dataType) {
  auto location = getLocationFromPropertyAddress(propertyAddress);
  BOOST_REQUIRE_MESSAGE(location, "could not find location for " + propertyAddress);
  location->lock();

  auto property = getDoocsProperty<D_fct>(propertyAddress);
  BOOST_REQUIRE_MESSAGE(property, "could not find property for " + propertyAddress);
  BOOST_CHECK(property->data_type() == dataType);

  location->unlock();
}

#define CHECK_WITH_TIMEOUT(...)                                                                                        \
  {                                                                                                                    \
    static const size_t nIterations = 10000;                                                                           \
    size_t local_index_i = 0;                                                                                          \
    for(; local_index_i < nIterations; ++local_index_i) {                                                              \
      if(__VA_ARGS__) {                                                                                                \
        std::cout << "ok after " << local_index_i << std::endl;                                                        \
        break;                                                                                                         \
      }                                                                                                                \
      usleep(100);                                                                                                     \
    }                                                                                                                  \
    if(local_index_i == nIterations) BOOST_CHECK(__VA_ARGS__);                                                         \
  }

template<class T>
void checkWithTimeout(std::function<T()> accessorFunction, T referenceValue, size_t nIterations = 10000,
    size_t microSecondsPerIteration = 100) {
  for(size_t i = 0; i < nIterations; ++i) {
    if(accessorFunction() == referenceValue) {
      std::cout << "test OK after " << i << " iterations" << std::endl;
      return;
    }
    usleep(microSecondsPerIteration);
  }

  std::stringstream errorMessage;
  errorMessage << "accessor function failed after " << nIterations << ", expected value is " << referenceValue
               << ", current value is " << accessorFunction();
  BOOST_ERROR(errorMessage.str());
}

void checkSpectrum(std::string const& propertyAddress, bool expected_has_history = true,
    bool expected_is_writeable = true, float expected_start = 0.0, float expected_increment = 1.0) {
  checkDoocsProperty<D_spectrum>(propertyAddress, expected_has_history, expected_is_writeable);

  CHECK_WITH_TIMEOUT(std::fabs(readSpectrumStart(propertyAddress) - expected_start) < 0.001);
  CHECK_WITH_TIMEOUT(std::fabs(readSpectrumIncrement(propertyAddress) - expected_increment) < 0.001);
}

#endif // SERVER_BASED_TEST_TOOLS_H
