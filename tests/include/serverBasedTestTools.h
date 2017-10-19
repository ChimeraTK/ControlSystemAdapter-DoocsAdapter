#ifndef SERVER_BASED_TEST_TOOLS_H
#define SERVER_BASED_TEST_TOOLS_H

#include <eq_fct.h>
#include "basenameFromAddress.h"

// these constants don't exist in DOOCS, although they should. We define them here for better code readability
static const int ACCESS_RO = 0; // read only
static const int ACCESS_RW = 1; // read/write

template<class DOOCS_T>
void checkHistory(DOOCS_T * property, bool expected_has_history){
  bool has_history = property->get_histPointer();
  BOOST_CHECK_MESSAGE( has_history == expected_has_history, "History on/off wrong for "+ property->basename() +". Should be " + (expected_has_history?"true":"false"));
}

template<>
void checkHistory(D_spectrum * /*property*/, bool){
  #warning FIXME implement check on D_spectrum history
}
#warning FIXME implement check on array histories
template<> void checkHistory(D_bytearray * /*property*/, bool){}
template<> void checkHistory(D_shortarray * /*property*/, bool){}
template<> void checkHistory(D_intarray * /*property*/, bool){}
template<> void checkHistory(D_longarray * /*property*/, bool){}
template<> void checkHistory(D_floatarray * /*property*/, bool){}
template<> void checkHistory(D_doublearray * /*property*/, bool){}

template<class DOOCS_T>
void checkDoocsProperty(std::string const & propertyAddress, bool expected_has_history =  true, bool expected_is_writeable =true){
  // copied from DoocsServerTestHelper::doocsGet
  EqAdr ad;
  EqData ed, res;
  // obtain location pointer
  ad.adr(propertyAddress.c_str());
  EqFct *eqFct = eq_get(&ad);
  BOOST_REQUIRE_MESSAGE( eqFct, "Could not get location for property "+propertyAddress);

  auto propertyName = ChimeraTK::basenameFromAddress(propertyAddress);
  DOOCS_T * property = dynamic_cast<DOOCS_T *>(eqFct->find_property(propertyName));
  BOOST_REQUIRE_MESSAGE(property, "Could not find property " + propertyName + " (address "<< propertyAddress <<"), or property has unexpected type.");

  checkHistory(property, expected_has_history);

  std::stringstream errorMessage;
  errorMessage << "Access rights not correct for '" << propertyAddress << "': access word is "
               << property->get_access() << ", expected " << (expected_is_writeable?ACCESS_RW:ACCESS_RO);
  if (expected_is_writeable){
    BOOST_CHECK_MESSAGE( property->get_access() == ACCESS_RW , errorMessage.str());
  }else{
    BOOST_CHECK( property->get_access() == ACCESS_RO );    
  }
}

void checkSpectrum(std::string const & propertyAddress, bool expected_has_history =  true, bool expected_is_writeable =true, float expected_start = 0.0, float expected_increment = 1.0){
  checkDoocsProperty<D_spectrum>(propertyAddress, expected_has_history, expected_is_writeable);

  EqAdr ad;
  EqData ed, res;
  // obtain location pointer
  ad.adr(propertyAddress.c_str());
  EqFct *eqFct = eq_get(&ad);
  BOOST_REQUIRE_MESSAGE( eqFct, "Could not get location for property "+propertyAddress);

  auto propertyName = ChimeraTK::basenameFromAddress(propertyAddress);
  D_spectrum * spectrum = dynamic_cast< D_spectrum *>(eqFct->find_property(propertyName));
  BOOST_REQUIRE_MESSAGE(spectrum, "Could not find property " + propertyName + " (address "<< propertyAddress <<"), or property has unexpected type.");

  std::cout << "The magic 4 " << spectrum->spec_time() << ", "
            << spectrum->spec_start() << ", "
            << spectrum->spec_inc() << ", "
            << spectrum->spec_status() << std::endl;
  BOOST_CHECK( std::fabs(spectrum->spec_start() - expected_start) < 0.001 );
  BOOST_CHECK( std::fabs(spectrum->spec_inc() - expected_increment) < 0.001 );
}

#endif // SERVER_BASED_TEST_TOOLS_H
