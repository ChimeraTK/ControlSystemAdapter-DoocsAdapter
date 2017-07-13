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
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsProperty(typename ProcessVariable::SharedPtr & processVariable) {
    // the DoocsProcessArray needs the real ProcessScalar type, not just ProcessVariable
    typename ProcessArray<T>::SharedPtr processArray
      = boost::dynamic_pointer_cast< ProcessArray<T> >(processVariable);
    if (!processArray){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the wrong type ")
				  + processVariable->getValueType().name());
    }

    assert(processArray->getNumberOfChannels() == 1);
    if(processArray->getNumberOfSamples() > 1 ) {
      return boost::shared_ptr<D_fct>( new DoocsProcessArray<T>(_eqFct, processArray, *_syncUtility) );
    }
    else {
      // Histories seem to be supported by DOOCS only for property names shorter than 64 characters, so disable history for longer names.
      // The DOOCS property name is the variable name without the location name and the separating slash between location and property name.
      if(processArray->getName().length() - 1 - std::strlen(_eqFct->name_str()) <= 64) {
        return boost::shared_ptr<D_fct>( new DoocsProcessScalar<T, DOOCS_T, DOOCS_VALUE_T>(_eqFct, processArray, *_syncUtility) );
      }
      else {
        return boost::shared_ptr<D_fct>( new DoocsProcessScalar<T, DOOCS_T, DOOCS_VALUE_T>(processArray, _eqFct, *_syncUtility) );
      }
    }
  }

  template<>
  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsProperty<std::string, D_string, std::string>(
            boost::shared_ptr<ProcessVariable> & processVariable) {
    // the DoocsProcessArray needs the real ProcessScalar type, not just ProcessVariable
    boost::shared_ptr<ProcessArray<std::string>> processArray
      = boost::dynamic_pointer_cast< ProcessArray<std::string> >(processVariable);
    if (!processArray){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the wrong type ")
                                  + processVariable->getValueType().name());
    }
    
    assert(processArray->getNumberOfChannels() == 1);
    assert(processArray->getNumberOfSamples() == 1);    // array of strings is not supported
    return boost::shared_ptr<D_fct>( new DoocsProcessScalar<std::string, D_string, std::string>(_eqFct, processArray, *_syncUtility) );
  }

 boost::shared_ptr<D_fct> DoocsPVFactory::create( ProcessVariable::SharedPtr & processVariable ){
    std::type_info const & valueType = processVariable->getValueType();

    // Unfortunately we need a big if/else block to hard-code the template
    // parameter. The value type in only known at run time,
    // but the template parameter has to be known at compile time.
    if (valueType == typeid(int8_t)) {
      return createDoocsProperty<int8_t, D_int, int>(processVariable);
    } else if (valueType == typeid(uint8_t)) {
      return createDoocsProperty<uint8_t, D_int, int>(processVariable);
    } else if (valueType == typeid(int16_t)) {
      return createDoocsProperty<int16_t, D_int, int>(processVariable);
    } else if (valueType == typeid(uint16_t)) {
      return createDoocsProperty<uint16_t, D_int, int>(processVariable);
    } else if (valueType == typeid(int32_t)) {
      return createDoocsProperty<int32_t, D_int, int>(processVariable);
    } else if (valueType == typeid(uint32_t)) {
      return createDoocsProperty<uint32_t, D_int, int>(processVariable);
    } else if (valueType == typeid(float)) {
      return createDoocsProperty<float, D_float, float>(processVariable);
    } else if (valueType == typeid(double)) {
      return createDoocsProperty<double, D_double, double>(processVariable);
    } else if (valueType == typeid(std::string)) {
      return createDoocsProperty<std::string, D_string, std::string>(processVariable);
    } else {
      throw std::invalid_argument("unsupported value type");
    }

   
}

}// namespace

