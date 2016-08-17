#define BOOST_TEST_MODULE DoocsProcessArrayTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "DoocsProcessArray.h"
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>

#include "emptyServerFunctions.h"

#include <limits>
#include <sstream>

using namespace boost::unit_test_framework;
using namespace ChimeraTK;

template <typename T>
class TestableDoocsArray : public DoocsProcessArray<T>{
public:
  TestableDoocsArray( EqFct * const eqFct,
		      boost::shared_ptr< typename ChimeraTK::ProcessArray<T> > const & processArray,
		      ControlSystemSynchronizationUtility & syncUtility)
    : DoocsProcessArray<T>( eqFct, processArray, syncUtility){}

  void sendToDevice(){
    DoocsProcessArray<T>::sendToDevice();
  }
	
};

// use boost meta-programming to use test case templates
// The list of types is an mpl type
typedef boost::mpl::list<int32_t, uint32_t,
			 int16_t, uint16_t,
			 int8_t, uint8_t,
			 float, double> simple_test_types;

BOOST_AUTO_TEST_SUITE( DoocsProcessArrayTestSuite )

BOOST_AUTO_TEST_CASE_TEMPLATE( toDeviceTest, T, simple_test_types ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);
  
  static const size_t arraySize = 8;
  boost::shared_ptr< ProcessArray< T> > deviceVariable =
    devManager->createProcessArray<T>(
      controlSystemToDevice, "toDeviceVariable", arraySize, 0 /*start value*/);
  boost::shared_ptr< ProcessArray<T> > controlSystemVariable = 
    csManager->getProcessArray<T>("toDeviceVariable");

  // Write to the doocs array and send it.
  // We use the 'testable' version which exposes sendToDevice, which otherwise is 
  // protected.
  TestableDoocsArray<T> doocsArray( NULL, controlSystemVariable, syncUtil );

  // create unique signature for each template parameter
  // negative factor for signed values
  T sign = (std::numeric_limits<T>::is_signed ? -1 : 1 );
  // integer size offset for integer, fractional offset for floating type
  T offset = (std::numeric_limits<T>::is_integer ? sizeof(T) : 1./sizeof(T) );
  
  for (size_t i =0; i < arraySize; ++i){
    doocsArray.fill_spectrum(i, sign*static_cast<T>(i*i) + offset);
  }
  doocsArray.sendToDevice();

  // receive on the device side and check that the value has arrived
  deviceVariable->receive();
  
  std::vector<T> & deviceVector = deviceVariable->get();
  for (size_t i =0; i < arraySize; ++i){
    std::stringstream errorMessage;
    errorMessage << "i = " <<i<< ", deviceVector[i] = " 
		 <<  deviceVector[i]
		 << " expected " << sign*static_cast<T>(i*i) + offset;
    BOOST_CHECK_MESSAGE( deviceVector[i] == sign*static_cast<T>(i*i) + offset,
			 errorMessage.str() );
  }

}

BOOST_AUTO_TEST_CASE_TEMPLATE( fromDeviceTest, T, simple_test_types ){
  std::pair<boost::shared_ptr<ControlSystemPVManager>,
	    boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;

  ControlSystemSynchronizationUtility syncUtil(csManager);

  static const size_t arraySize = 8;
  typename ProcessArray<T>::SharedPtr deviceVariable =
    devManager->createProcessArray<T>(
      deviceToControlSystem, "fromDeviceVariable", arraySize, 0);
  typename ProcessArray<T>::SharedPtr controlSystemVariable = 
    csManager->getProcessArray<T>("fromDeviceVariable");

  // initialise the doocs array
  DoocsProcessArray<T> doocsArray( NULL, controlSystemVariable, syncUtil );
  for (size_t i =0; i < arraySize; ++i){
    doocsArray.fill_spectrum(i, 0);
  }

  // prepare the device side and send it
  T sign = (std::numeric_limits<T>::is_signed ? -1 : 1 );
  T offset = (std::numeric_limits<T>::is_integer ? sizeof(T) : 1./sizeof(T) );

  std::vector<T> & deviceVector = deviceVariable->get();
  for (size_t i =0; i < arraySize; ++i){
    deviceVector[i] = (sign*static_cast<T>(i*i) + offset) ;
  }
  deviceVariable->send();

  // everything should still be 0 on the CS side
  std::vector<T> & csVector = controlSystemVariable->get();
  for (size_t i =0; i < arraySize; ++i){
    BOOST_CHECK( csVector[i] == 0 );
    BOOST_CHECK( doocsArray.read_spectrum(i) == 0 );
  }

  syncUtil.receiveAll();
  // The actual vector buffer has changed. We have to get the new reference.
  csVector = controlSystemVariable->get();  
  for (size_t i =0; i < arraySize; ++i){
    BOOST_CHECK( csVector[i] == sign*static_cast<T>(i*i) + offset );
    BOOST_CHECK( doocsArray.read_spectrum(i) == sign*static_cast<T>(i*i) + offset );
  }

}

BOOST_AUTO_TEST_SUITE_END()
