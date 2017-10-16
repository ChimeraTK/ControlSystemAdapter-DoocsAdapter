#include <boost/make_shared.hpp>

#include "DoocsProcessScalar.h"
#include "DoocsProcessArray.h"
#include "DoocsSpectrum.h"
#include <d_fct.h>

#include "DoocsPVFactory.h"
#include "splitStringAtFirstSlash.h"
#include <ChimeraTK/ControlSystemAdapter/TypeChangingDecorator.h>

namespace ChimeraTK {

  DoocsPVFactory::DoocsPVFactory(EqFct * const eqFct,
                                 DoocsUpdater & updater,
                                 boost::shared_ptr<ControlSystemPVManager> const & csPVManager) 
    : _eqFct(eqFct), _updater(updater), _controlSystemPVManager(csPVManager) {
      assert(eqFct != nullptr);
  }

  // Fixme: is AutoPropertyDescription ok, or to we need IntDescripton, DoubleDescription etc.
  template<class DOOCS_PRIMITIVE_T, class DOOCS_T>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar(AutoPropertyDescription const & propertyDescription, DecoratorType decoratorType) {
    auto processVariable = _controlSystemPVManager->getProcessVariable(propertyDescription.source);

    // the DoocsProcessScalar needs the real ProcessScalar type, not just ProcessVariable
    auto processArray =  getDecorator<DOOCS_PRIMITIVE_T>( *processVariable, decoratorType);
    
    assert(processArray->getNumberOfChannels() == 1);
    boost::shared_ptr<D_fct> doocsPV;
    // Histories seem to be supported by DOOCS only for property names shorter than 64 characters, so disable history for longer names.
    // The DOOCS property name is the variable name without the location name and the separating slash between location and property name.
    if(propertyDescription.name.length() > 64) {
      std::cerr << "WARNING: Disabling history for " << processArray->getName() << ". Name is too long." << std::endl;
      doocsPV.reset( new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(propertyDescription.name.c_str(), _eqFct, processArray, _updater) );
    }
    else{
      if (propertyDescription.hasHistory){
        // version with history: EqFtc first
        doocsPV.reset( new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(_eqFct, propertyDescription.name.c_str(), processArray, _updater) );
      }else{
        // version without history: name first
        doocsPV.reset( new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(propertyDescription.name.c_str(), _eqFct, processArray, _updater) );
      }
    }// if name too long

    // set read only mode if configures in the xml file or for output variables
    if (!processArray->isWriteable() || !propertyDescription.isWriteable){
      doocsPV->set_ro_access();
    }
    
    return doocsPV;
  }

  template<>
  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar<std::string, D_string>(
    AutoPropertyDescription const & propertyDescription, DecoratorType /*decoratorType*/){

    auto processVariable = _controlSystemPVManager->getProcessVariable(propertyDescription.source);
   
    // FIXME: Use a decorator, but this has to be tested and implemented for strings first
    // the DoocsProcessArray needs the real ProcessScalar type, not just ProcessVariable
    boost::shared_ptr< mtca4u::NDRegisterAccessor<std::string> > processArray
      = boost::dynamic_pointer_cast< mtca4u::NDRegisterAccessor<std::string> >(processVariable);
    if (!processArray){
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the wrong type ")
                                  + processVariable->getValueType().name());
    }
    
    assert(processArray->getNumberOfChannels() == 1);
    assert(processArray->getNumberOfSamples() == 1);    // array of strings is not supported
    boost::shared_ptr<D_fct> doocsPV( new DoocsProcessScalar<std::string, D_string>(_eqFct, propertyDescription.name.c_str(), processArray, _updater) );

    // set read only mode if configures in the xml file or for output variables
    if (!processArray->isWriteable() || !propertyDescription.isWriteable){
      doocsPV->set_ro_access();
    }
    
    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsSpectrum(AutoPropertyDescription const & spectrumDescription) {
    auto processVariable = _controlSystemPVManager->getProcessVariable(spectrumDescription.source);

    //    assert(processArray->getNumberOfChannels() == 1);
    boost::shared_ptr<D_fct> doocsPV( new DoocsSpectrum(_eqFct, spectrumDescription.name, getDecorator<float>(*processVariable), _updater) );

    // set read only mode if configures in the xml file or for output variables
    if (!processVariable->isWriteable() || !spectrumDescription.isWriteable){
      doocsPV->set_ro_access();
    }

    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::autoCreate( std::shared_ptr<PropertyDescription> const & propertyDescription ){
    // do auto creation
    auto autoPropertyDescription = std::static_pointer_cast<AutoPropertyDescription>(propertyDescription);

    auto pvName = autoPropertyDescription->source;
    auto processVariable = _controlSystemPVManager->getProcessVariable(pvName);

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
        return createDoocsScalar<int32_t, D_int>(*autoPropertyDescription, DecoratorType::C_style_conversion);
      } else if (valueType == typeid(uint8_t)) {
        return createDoocsScalar<int32_t, D_int>(*autoPropertyDescription, DecoratorType::C_style_conversion);
      } else if (valueType == typeid(int16_t)) {
        return createDoocsScalar<int32_t, D_int>(*autoPropertyDescription, DecoratorType::C_style_conversion);
      } else if (valueType == typeid(uint16_t)) {
        return createDoocsScalar<int32_t, D_int>(*autoPropertyDescription, DecoratorType::C_style_conversion);
      } else if (valueType == typeid(int32_t)) {
        return createDoocsScalar<int32_t, D_int>(*autoPropertyDescription, DecoratorType::C_style_conversion);
      } else if (valueType == typeid(uint32_t)) {
        return createDoocsScalar<int32_t, D_int>(*autoPropertyDescription, DecoratorType::C_style_conversion);
      } else if (valueType == typeid(float)) {
        return createDoocsScalar<float, D_float>(*autoPropertyDescription, DecoratorType::C_style_conversion);
      } else if (valueType == typeid(double)) {
        return createDoocsScalar<double, D_double>(*autoPropertyDescription, DecoratorType::range_checking);
      } else if (valueType == typeid(std::string)) {
        return createDoocsScalar<std::string, D_string>(*autoPropertyDescription, DecoratorType::range_checking);
      } else {
        throw std::invalid_argument("unsupported value type");
      }
    }else{ //nSamples > 1
      if (valueType == typeid(int8_t)) {
        return  typedCreateDoocsArray<int8_t, D_bytearray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(uint8_t)) {
        return  typedCreateDoocsArray<int8_t, D_bytearray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(int16_t)) {
        return  typedCreateDoocsArray<int16_t, D_shortarray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(uint16_t)) {
        return  typedCreateDoocsArray<int16_t, D_shortarray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(int32_t)) {
        return  typedCreateDoocsArray<int32_t, D_intarray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(uint32_t)) {
        return  typedCreateDoocsArray<int32_t, D_intarray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(int64_t)) {
        return  typedCreateDoocsArray<long long int, D_longarray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(uint64_t)) {
        return  typedCreateDoocsArray<long long int, D_longarray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(float)) {
        return  typedCreateDoocsArray<float, D_floatarray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else if (valueType == typeid(double)) {
        return  typedCreateDoocsArray<double, D_doublearray>(ArrayDescription(*autoPropertyDescription, ArrayDescription::DataType::Byte));
      } else {
        throw std::invalid_argument("unsupported value type");
      }
    }
 }

  template<class DOOCS_PRIMITIVE_T, class DOOCS_T>
  boost::shared_ptr<D_fct> DoocsPVFactory::typedCreateDoocsArray( ArrayDescription const & arrayDescription){
    auto processVariable = _controlSystemPVManager->getProcessVariable(arrayDescription.source);

    boost::shared_ptr<D_fct> doocsPV( new DoocsProcessArray< DOOCS_T, DOOCS_PRIMITIVE_T>(_eqFct, arrayDescription.name, getDecorator<DOOCS_PRIMITIVE_T>(*processVariable), _updater) );

    // set read only mode if configures in the xml file or for output variables
    if (!processVariable->isWriteable() || !arrayDescription.isWriteable){
      doocsPV->set_ro_access();
    }

    return doocsPV;
  }
  
  boost::shared_ptr<D_fct>  DoocsPVFactory::createDoocsArray(  std::shared_ptr<ArrayDescription> const & arrayDescription ){
    if(arrayDescription->dataType == ArrayDescription::DataType::Auto){
      // leave the desision which array to produce to the auto creation algorithm. We need it there anyway
      //FIXME: This does not produce arrays of length 1 because it will produce a scalar
      return autoCreate(arrayDescription);
    }else if(arrayDescription->dataType == ArrayDescription::DataType::Byte){
      return typedCreateDoocsArray<int8_t, D_bytearray>(*arrayDescription);
    }else if(arrayDescription->dataType == ArrayDescription::DataType::Short){
      return typedCreateDoocsArray<int16_t, D_shortarray>(*arrayDescription);
    }else if(arrayDescription->dataType == ArrayDescription::DataType::Int){
      return typedCreateDoocsArray<int32_t, D_intarray>(*arrayDescription);
    }else if(arrayDescription->dataType == ArrayDescription::DataType::Long){
      return typedCreateDoocsArray<long long int, D_longarray>(*arrayDescription);
    }else if(arrayDescription->dataType == ArrayDescription::DataType::Float){
      return typedCreateDoocsArray<float, D_floatarray>(*arrayDescription);
    }else if(arrayDescription->dataType == ArrayDescription::DataType::Double){
      return typedCreateDoocsArray<double, D_doublearray>(*arrayDescription);
    }else{
      throw std::logic_error("DoocsPVFactory does not implement a data type it should!");
    }

  }

    boost::shared_ptr<D_fct>  DoocsPVFactory::create( std::shared_ptr<PropertyDescription> const & propertyDescription ){
    auto & requestedType = propertyDescription->type();
    if (requestedType == typeid(AutoPropertyDescription)){
      std::cout << "creating auto for " << propertyDescription->name << std::endl;
      return autoCreate(propertyDescription);
    }else if (requestedType == typeid(SpectrumDescription)){
      std::cout << "creating Spectrum for " << propertyDescription->name << std::endl;
      return createDoocsSpectrum(*std::static_pointer_cast<SpectrumDescription>(propertyDescription));
    }else if (requestedType == typeid(ArrayDescription)){
      std::cout << "creating Array for " << propertyDescription->name << std::endl;
      return createDoocsArray(std::static_pointer_cast<ArrayDescription>(propertyDescription));
    }else{
      throw std::invalid_argument("Sorry, your type is not supported yet.");
    }
  }
  
}// namespace

