#include <boost/make_shared.hpp>

#include "DoocsProcessScalar.h"
#include <d_fct.h>
//#include <DoocsProcessArray.h>

#include "DoocsPVFactory.h"

namespace mtca4u {

  DoocsPVFactory::DoocsPVFactory(EqFct * const eqFct,
				 boost::shared_ptr<ControlSystemSynchronizationUtility> const & syncUtility) 
    : _eqFct(eqFct), _syncUtility(syncUtility) {
  }

  template<class T, class DOOCS_T>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar( 
    typename ProcessVariable::SharedPtr & processVariable){
    // the DoocsProcessScalar needs the 
    typename ProcessScalar<T>::SharedPtr processScalar 
      = boost::dynamic_pointer_cast< ProcessScalar<T> >(processVariable);
    if (!processScalar){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsScalar : processScalar is of the wrong type ")
				  + processVariable->getValueType().name());
    }
    
    return boost::shared_ptr<D_fct>( new DoocsProcessScalar<T, DOOCS_T>(_eqFct, processScalar, *_syncUtility) );
  }

//  template<class T, class DOOCS_T>
//  typename ControlSystemProcessScalar<T>::SharedPtr DoocsPVFactory::createProcessScalarInternal(
//      const std::string& name) {
//    return boost::make_shared< DoocsProcessScalar<T, DOOCS_T> >(name,  _eqFct, _pvManager);
//  }

  boost::shared_ptr<D_fct> DoocsPVFactory::create( ProcessVariable::SharedPtr & processVariable ){
    std::type_info const & valueType = processVariable->getValueType();

    if( processVariable->isArray() ){ // it's a scalar, call the createDoocsArray method
      throw std::runtime_error("Not implemented yet");
    }
    else{ // it's a scalar, call the createScalar method
      if (valueType == typeid(int8_t)) {
	return createDoocsScalar<int8_t, D_int>(processVariable);
      } else if (valueType == typeid(uint8_t)) {
	return createDoocsScalar<uint8_t, D_int>(processVariable);
      } else if (valueType == typeid(int16_t)) {
	return createDoocsScalar<int16_t, D_int>(processVariable);
      } else if (valueType == typeid(uint16_t)) {
	return createDoocsScalar<uint16_t, D_int>(processVariable);
      } else if (valueType == typeid(int32_t)) {
	return createDoocsScalar<int32_t, D_int>(processVariable);
      } else if (valueType == typeid(uint32_t)) {
	return createDoocsScalar<uint32_t, D_int>(processVariable);
      } else if (valueType == typeid(float)) {
	return createDoocsScalar<float, D_float>(processVariable);
      } else if (valueType == typeid(double)) {
	return createDoocsScalar<double, D_double>(processVariable);
      } else {
	throw std::invalid_argument("unsupported value type");
      }
    }// else isArray
  }

}// namespace

