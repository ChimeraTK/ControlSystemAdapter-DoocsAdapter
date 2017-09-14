#include <boost/make_shared.hpp>

#include "DoocsProcessScalar.h"
#include "DoocsProcessArray.h"
#include "DoocsSpectrum.h"
#include <d_fct.h>

#include "DoocsPVFactory.h"
#include "splitStringAtFirstSlash.h"
#include "VariableMapper.h"
#include <ChimeraTK/ControlSystemAdapter/TypeChangingDecorator.h>

namespace ChimeraTK {

  DoocsPVFactory::DoocsPVFactory(EqFct * const eqFct,
				 boost::shared_ptr<ControlSystemSynchronizationUtility> const & syncUtility, boost::shared_ptr<ControlSystemPVManager> const & csPVManager) 
    : _eqFct(eqFct), _syncUtility(syncUtility), _controlSystemPVManager(csPVManager) {
      assert(eqFct != nullptr);
  }

  template<class T, class DOOCS_T>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar(typename ProcessVariable::SharedPtr & processVariable) {
    // the DoocsProcessArray needs the real ProcessScalar type, not just ProcessVariable
    typename boost::shared_ptr< mtca4u::NDRegisterAccessor<T> > processArray
      = boost::dynamic_pointer_cast< mtca4u::NDRegisterAccessor<T> >(processVariable);
    if (!processArray){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the wrong type ")
				  + processVariable->getValueType().name());
    }

    auto propertyDescription = VariableMapper::getInstance().getAllProperties().at(processVariable->getName());
    // FIXME: This has to go for scalars
    auto autoPropertyDescription = std::dynamic_pointer_cast<AutoPropertyDescription>(propertyDescription);
    
    assert(processArray->getNumberOfChannels() == 1);
    boost::shared_ptr<D_fct> doocsPV;
    // Histories seem to be supported by DOOCS only for property names shorter than 64 characters, so disable history for longer names.
    // The DOOCS property name is the variable name without the location name and the separating slash between location and property name.
    if(propertyDescription->name.length() > 64) {
      std::cerr << "WARNING: Disabling history for " << processArray->getName() << ". Name is too long." << std::endl;
      doocsPV.reset( new DoocsProcessScalar<T, DOOCS_T>(propertyDescription->name.c_str(), _eqFct, processArray, *_syncUtility) );
    }
    else{
      if (autoPropertyDescription && autoPropertyDescription->hasHistory){
        // version with history: EqFtc first
        doocsPV.reset( new DoocsProcessScalar<T, DOOCS_T>(_eqFct, propertyDescription->name.c_str(), processArray, *_syncUtility) );
      }else{
        // version without history: name first
        doocsPV.reset( new DoocsProcessScalar<T, DOOCS_T>(propertyDescription->name.c_str(), _eqFct, processArray, *_syncUtility) );
      }
    }// if name too long

    // FIXME: Make it scalar and put it into one if query
    if (autoPropertyDescription && !(autoPropertyDescription->isWriteable)){
      doocsPV->set_ro_access();
    }
    
    // set read only mode if configures in the xml file or for output variables
    if (!processArray->isWriteable()){// || !propertyDescription.isWriteable){
      doocsPV->set_ro_access();
    }
    
    return doocsPV;
  }

  template<>
  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar<std::string, D_string>(
            boost::shared_ptr<ProcessVariable> & processVariable) {
    // the DoocsProcessArray needs the real ProcessScalar type, not just ProcessVariable
    boost::shared_ptr< mtca4u::NDRegisterAccessor<std::string> > processArray
      = boost::dynamic_pointer_cast< mtca4u::NDRegisterAccessor<std::string> >(processVariable);
    if (!processArray){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the wrong type ")
                                  + processVariable->getValueType().name());
    }
    
    auto propertyDescription = VariableMapper::getInstance().getAllProperties().at(processVariable->getName());

    assert(processArray->getNumberOfChannels() == 1);
    assert(processArray->getNumberOfSamples() == 1);    // array of strings is not supported
    boost::shared_ptr<D_fct> doocsPV( new DoocsProcessScalar<std::string, D_string>(_eqFct, propertyDescription->name.c_str(), processArray, *_syncUtility) );

    //fixme: factor this out into another function. Will be needed many times
    auto autoPropertyDescription = std::dynamic_pointer_cast<AutoPropertyDescription>(propertyDescription);
    // FIXME: This has to go for scalars
    // FIXME: Make it scalar and put it into one if query
    if (autoPropertyDescription && !(autoPropertyDescription->isWriteable)){
      doocsPV->set_ro_access();
    }
    
    // set read only mode if configures in the xml file or for output variables
    if (!processArray->isWriteable()){// || !propertyDescription.isWriteable){
      doocsPV->set_ro_access();
    }
    
    return doocsPV;
  }

  template<class T>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsSpectrum(typename ProcessVariable::SharedPtr & processVariable) {
    // the DoocsProcessArray needs the real type, not just ProcessVariable
    typename boost::shared_ptr< mtca4u::NDRegisterAccessor<T> > processArray
      = boost::dynamic_pointer_cast< mtca4u::NDRegisterAccessor<T> >(processVariable);
    if (!processArray){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the wrong type ")
				  + processVariable->getValueType().name());
    }

    auto propertyDescription = VariableMapper::getInstance().getAllProperties().at(processVariable->getName());
    // FIXME: This has to go for scalars
    auto autoPropertyDescription = std::dynamic_pointer_cast<AutoPropertyDescription>(propertyDescription);
    
    assert(processArray->getNumberOfChannels() == 1);
    boost::shared_ptr<D_fct> doocsPV( new DoocsSpectrum(_eqFct, propertyDescription->name, getDecorator<float>(*processArray), *_syncUtility) );

    // FIXME: Make it scalar and put it into one if query
    if (autoPropertyDescription && !(autoPropertyDescription->isWriteable)){
      doocsPV->set_ro_access();
    }

    // set read only mode if configures in the xml file or for output variables
    if (!processArray->isWriteable()){// || !propertyDescription.isWriteable){
      doocsPV->set_ro_access();
    }

    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::autoCreate( std::shared_ptr<PropertyDescription> const & propertyDescription ){
    // do auto creation
    auto pvName = std::static_pointer_cast<AutoPropertyDescription>(propertyDescription)->source;
    auto pv = _controlSystemPVManager->getProcessVariable(pvName);
    // fixme: merge the create stuff in here and remove the create function.
    return create( pv );
  }
  
  boost::shared_ptr<D_fct> DoocsPVFactory::create( ProcessVariable::SharedPtr & processVariable ){
    std::type_info const & valueType = processVariable->getValueType();
    // We use an int accessor decorator just to have some type.
    // All we need to do here is to determine the number of samples. No need to write an if/then/else
    // on the type, or a boot::fustion::for_each, which is in the decorator factory.
    // Unfortynately we need different impl_type -> doocs_type decision for scalars and arrays,
    // so we have to know this here.
    auto intDecoratedPV = ChimeraTK::getDecorator<int>( *processVariable );
    auto nSamples = intDecoratedPV->getNumberOfSamples();

    /*  TODO: 
        - create functions "createDoocsArray" and "createDoocsSpectrum"
        - first use spectrum here for 1D, then switch to array (tests need to be adapted)
        - create spectrum, array and d_int/float/double upon request from 1d (scalar for D_array and 1D)
    */
    // Unfortunately we need a big if/else block to hard-code the template
    // parameter. The value type in only known at run time,
    // but the template parameter has to be known at compile time.
    if (nSamples == 1){
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
      } else if (valueType == typeid(std::string)) {
        return createDoocsScalar<std::string, D_string>(processVariable);
      } else {
        throw std::invalid_argument("unsupported value type");
      }
    }else{ //nSamples > 1
      if (valueType == typeid(int8_t)) {
        return createDoocsSpectrum<int8_t>(processVariable);
      } else if (valueType == typeid(uint8_t)) {
        return createDoocsSpectrum<uint8_t>(processVariable);
      } else if (valueType == typeid(int16_t)) {
        return createDoocsSpectrum<int16_t>(processVariable);
      } else if (valueType == typeid(uint16_t)) {
        return createDoocsSpectrum<uint16_t>(processVariable);
      } else if (valueType == typeid(int32_t)) {
        return createDoocsSpectrum<int32_t>(processVariable);
      } else if (valueType == typeid(uint32_t)) {
        return createDoocsSpectrum<uint32_t>(processVariable);
      } else if (valueType == typeid(float)) {
        return createDoocsSpectrum<float>(processVariable);
      } else if (valueType == typeid(double)) {
        return createDoocsSpectrum<double>(processVariable);
      } else {
        throw std::invalid_argument("unsupported value type");
      }
    }
 }

  boost::shared_ptr<D_fct>  DoocsPVFactory::new_create( std::shared_ptr<PropertyDescription> const & propertyDescription ){
    auto & requestedType = propertyDescription->type();
    if (requestedType == typeid(AutoPropertyDescription)){
      // do auto creation
      auto pvName = std::static_pointer_cast<AutoPropertyDescription>(propertyDescription)->source;
      auto pv = _controlSystemPVManager->getProcessVariable(pvName);
      return create(pv);
    }else{
      throw std::invalid_argument("Sorry, your type is not supported yet.");
    }
  }
  
}// namespace

