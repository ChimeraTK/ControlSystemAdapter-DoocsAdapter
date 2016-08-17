#include <boost/make_shared.hpp>

#include "DoocsProcessScalar.h"
#include "DoocsProcessArray.h"
#include <d_fct.h>

#include "DoocsPVFactory.h"

namespace ChimeraTK {

  DoocsPVFactory::DoocsPVFactory(EqFct * const eqFct,
				 boost::shared_ptr<ControlSystemSynchronizationUtility> const & syncUtility) 
    : _eqFct(eqFct), _syncUtility(syncUtility) {
  }

  template<class T, class DOOCS_T, class DOOCS_VALUE_T>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar( 
    typename ProcessVariable::SharedPtr & processVariable){
    // the DoocsProcessScalar needs the real ProcessScalar type,
    // not just ProcessVariable
    typename ProcessScalar<T>::SharedPtr processScalar 
      = boost::dynamic_pointer_cast< ProcessScalar<T> >(processVariable);
    if (!processScalar){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsScalar : processScalar is of the wrong type ")
				  + processVariable->getValueType().name());
    }
    
    return boost::shared_ptr<D_fct>( new DoocsProcessScalar<T, DOOCS_T, DOOCS_VALUE_T>(_eqFct, processScalar, *_syncUtility) );
  }

  template<class T>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsArray( 
    typename ProcessVariable::SharedPtr & processVariable){
    // the DoocsProcessArray needs the real ProcessScalar type,
    // not just ProcessVariable
    typename ProcessArray<T>::SharedPtr processArray
      = boost::dynamic_pointer_cast< ProcessArray<T> >(processVariable);
    if (!processArray){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the wrong type ")
				  + processVariable->getValueType().name());
    }
    
    return boost::shared_ptr<D_fct>( 
      new DoocsProcessArray<T>(_eqFct, processArray, *_syncUtility) );
  }

 boost::shared_ptr<D_fct> DoocsPVFactory::create( ProcessVariable::SharedPtr & processVariable ){
    std::type_info const & valueType = processVariable->getValueType();

    if( processVariable->isArray() ){ 
      // it's an array, call the createDoocsArray method
      // Unfortunately we need a big if/else block to hard-code the template
      // parameter. The value type in only known at run time,
      // but the template parameter has to be known at compile time.
      if (valueType == typeid(int8_t)) {
	return createDoocsArray<int8_t>(processVariable);
      } else if (valueType == typeid(uint8_t)) {
	return createDoocsArray<uint8_t>(processVariable);
      } else if (valueType == typeid(int16_t)) {
	return createDoocsArray<int16_t>(processVariable);
      } else if (valueType == typeid(uint16_t)) {
	return createDoocsArray<uint16_t>(processVariable);
      } else if (valueType == typeid(int32_t)) {
	return createDoocsArray<int32_t>(processVariable);
      } else if (valueType == typeid(uint32_t)) {
	return createDoocsArray<uint32_t>(processVariable);
      } else if (valueType == typeid(float)) {
	return createDoocsArray<float>(processVariable);
      } else if (valueType == typeid(double)) {
	return createDoocsArray<double>(processVariable);
      } else {
	throw std::invalid_argument("unsupported value type");
      }
    }
    else{ // it's a scalar, call the createScalar method
      if (valueType == typeid(int8_t)) {
	return createDoocsScalar<int8_t, D_int, int>(processVariable);
      } else if (valueType == typeid(uint8_t)) {
	return createDoocsScalar<uint8_t, D_int, int>(processVariable);
      } else if (valueType == typeid(int16_t)) {
	return createDoocsScalar<int16_t, D_int, int>(processVariable);
      } else if (valueType == typeid(uint16_t)) {
	return createDoocsScalar<uint16_t, D_int, int>(processVariable);
      } else if (valueType == typeid(int32_t)) {
	return createDoocsScalar<int32_t, D_int, int>(processVariable);
      } else if (valueType == typeid(uint32_t)) {
	return createDoocsScalar<uint32_t, D_int, int>(processVariable);
      } else if (valueType == typeid(float)) {
	return createDoocsScalar<float, D_float, float>(processVariable);
      } else if (valueType == typeid(double)) {
	return createDoocsScalar<double, D_double, double>(processVariable);
      } else {
	throw std::invalid_argument("unsupported value type");
      }
    }// else isArray
  }

}// namespace

