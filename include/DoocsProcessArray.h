#ifndef __DOOCS_PROCESS_ARRAY_H__
#define __DOOCS_PROCESS_ARRAY_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include <ControlSystemAdapter/ProcessArray.h>
#include <ControlSystemAdapter/ProcessVariableListener.h>
#include <ControlSystemAdapter/ControlSystemSynchronizationUtility.h>

#include "splitStringAtFirstSlash.h"

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace mtca4u {
  
template <typename T>
class DoocsProcessArray :  public D_spectrum, public boost::noncopyable{
    
protected:

  /** A helper class to register notifications for DoocsProcessArrays
   */
  class DoocsArrayListener: public ProcessVariableListener{
    public:
      /** The constructor gets a pointer to a Doocs variable. As it is only used inside a 
       *  DoocsProcessVariable, we do not have to care about the scope of the pointer. It 
       *  will always be valid.
       */
      DoocsArrayListener(D_spectrum * spectrum): _spectrum(spectrum){
      }
    
      /** The notification that is executed updates of the doocs process variable
       */
      void notify(boost::shared_ptr< ProcessVariable > processVariable){
	// It is safe to static cast because the DoocsArrayListener is inside a 
	// DoocsProcessArray, which always holds a ProcessArray, never a ProcessScalar
	ProcessArray<T> & processArray = static_cast< ProcessArray<T> & >(*processVariable);
	
	// Brute force implementation. Works for all data types T.
	// always get a fresh reference
	std::vector<T> & processVector = processArray.get(); 
	size_t arraySize = processVector.size();
	for (size_t i=0; i < arraySize; ++i){
	  _spectrum->fill_spectrum(i, processVector[i]);
	}
      }
      
    private:
      D_spectrum * _spectrum;
  };

  boost::shared_ptr< ProcessArray<T> > _processArray;

  // Internal function which copies the content from the DOOCS container into the 
  // mtca4u ProcessArray and calls the send method. Factored out to allow unit testing.
  void sendToDevice(){
    /** Brute force implementation with a loop. Works for all data types.
     */
    // always get a fresh reference
    std::vector<T> & processVector = _processArray->get(); 
    size_t arraySize = processVector.size();
    for (size_t i=0; i < arraySize; ++i){
      processVector[i] = read_spectrum(i);
    }
    _processArray->send();
  }

public:
  DoocsProcessArray( EqFct * const eqFct,
		      boost::shared_ptr< typename mtca4u::ProcessArray<T> > const & processArray,
		      ControlSystemSynchronizationUtility & syncUtility)
    : D_spectrum( splitStringAtFirstSlash(processArray->getName()).second.c_str(),
		  processArray->get().size(), eqFct),
      _processArray(processArray) {
      syncUtility.addReceiveNotificationListener( processArray->getName(),
      ProcessVariableListener::SharedPtr( new DoocsArrayListener(this) ) );
    }

  /** Overload the set function which is called by DOOCS to inject sending to the device.
   */
  void set(EqAdr *eqAdr, EqData *data1, EqData *data2, EqFct *eqFct){
    D_spectrum::set(eqAdr, data1, data2, eqFct);
    sendToDevice();
  }

};

} // namespace mtca4u

#endif // __DOOCS_PROCESS_ARRAY_H__

