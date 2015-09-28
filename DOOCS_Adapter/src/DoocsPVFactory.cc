#include <boost/make_shared.hpp>

#if 0
#include "DoocsProcessScalar.h"
#include <d_fct.h>
//#include <DoocsProcessArray.h>

#include "DoocsPVFactory.h"

namespace mtca4u {

  DoocsPVFactory::DoocsPVFactory(EqFct *eqFct) : _eqFct(eqFct) {
  }

  template<class T, class DOOCS_T>
  typename ControlSystemProcessScalar<T>::SharedPtr DoocsPVFactory::createProcessScalarInternal( const std::string& name){
    return boost::make_shared< DoocsProcessScalar<T, DOOCS_T> >(name,  _eqFct, _pvManager.lock());
  }

//  template<class T, class DOOCS_T>
//  typename ControlSystemProcessScalar<T>::SharedPtr DoocsPVFactory::createProcessScalarInternal(
//      const std::string& name) {
//    return boost::make_shared< DoocsProcessScalar<T, DOOCS_T> >(name,  _eqFct, _pvManager);
//  }

  ControlSystemProcessVariable::SharedPtr DoocsPVFactory::createProcessScalar(
      const std::string& processVariableName, const std::type_info& valueType) {
    if (valueType == typeid(int8_t)) {
      return createProcessScalarInternal<int8_t, D_int>(processVariableName);
    } else if (valueType == typeid(uint8_t)) {
      return createProcessScalarInternal<uint8_t, D_int>(processVariableName);
    } else if (valueType == typeid(int16_t)) {
      return createProcessScalarInternal<int16_t, D_int>(processVariableName);
    } else if (valueType == typeid(uint16_t)) {
      return createProcessScalarInternal<uint16_t, D_int>(processVariableName);
    } else if (valueType == typeid(int32_t)) {
      return createProcessScalarInternal<int32_t, D_int>(processVariableName);
    } else if (valueType == typeid(uint32_t)) {
      return createProcessScalarInternal<uint32_t, D_int>(processVariableName);
    } else if (valueType == typeid(float)) {
      return createProcessScalarInternal<float, D_float>(processVariableName);
    } else if (valueType == typeid(double)) {
      return createProcessScalarInternal<double, D_double>(processVariableName);
    } else {
      throw std::invalid_argument("unsupported value type");
    }
  }

  template<class T>
  typename ControlSystemProcessArray<T>::SharedPtr createProcessArrayInternal(
      const std::string& name, size_t size) {
    return  ControlSystemProcessArray<T>::SharedPtr;
    //boost::make_shared<ManagedControlSystemProcessArray<T> >(name, size);
  }

  ControlSystemProcessVariable::SharedPtr DoocsPVFactory::createProcessArray(
    const std::string& /*processVariableName*/, size_t /*size*/,
    const std::type_info& /*elementType*/, bool /*swappable*/) {
//   if (elementType == typeid(int8_t)) {
//    return createProcessArrayInternal<int8_t>(processVariableName, size);
 //   } else if (elementType == typeid(uint8_t)) {
 //     return createProcessArrayInternal<uint8_t>(processVariableName, size);
 //   } else if (elementType == typeid(int16_t)) {
 //     return createProcessArrayInternal<int16_t>(processVariableName, size);
 //   } else if (elementType == typeid(uint16_t)) {
 //     return createProcessArrayInternal<uint16_t>(processVariableName, size);
 //   } else if (elementType == typeid(int32_t)) {
 //     return createProcessArrayInternal<int32_t>(processVariableName, size);
 //   } else if (elementType == typeid(uint32_t)) {
 //     return createProcessArrayInternal<uint32_t>(processVariableName, size);
 //   } else if (elementType == typeid(float)) {
 //     return createProcessArrayInternal<float>(processVariableName, size);
 //   } else if (elementType == typeid(double)) {
 //     return createProcessArrayInternal<double>(processVariableName, size);
    //   } else {
    //     throw std::invalid_argument("unsupported value type");
    //   }
 // }
    throw std::runtime_error("Not implemented yet");
  }

  void DoocsPVFactory::setDoocsPVManager( boost::shared_ptr<DoocsPVManager> doocsPVManager ){
    _pvManager = doocsPVManager;
  }

}// namespace

#endif //0
