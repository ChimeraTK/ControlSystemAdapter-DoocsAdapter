#include <boost/make_shared.hpp>

#include "DoocsProcessArray.h"
#include "DoocsProcessScalar.h"
#include "DoocsSpectrum.h"
#include <d_fct.h>

#include "DoocsPVFactory.h"
#include "splitStringAtFirstSlash.h"
#include <ChimeraTK/ControlSystemAdapter/TypeChangingDecorator.h>

namespace ChimeraTK {

DoocsPVFactory::DoocsPVFactory(
    EqFct *const eqFct, DoocsUpdater &updater,
    boost::shared_ptr<ControlSystemPVManager> const &csPVManager)
    : _eqFct(eqFct), _updater(updater), _controlSystemPVManager(csPVManager) {
  assert(eqFct != nullptr);
}

// Fixme: is AutoPropertyDescription ok, or to we need IntDescripton,
// DoubleDescription etc.
template <class DOOCS_PRIMITIVE_T, class DOOCS_T>
typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar(
    AutoPropertyDescription const &propertyDescription,
    DecoratorType decoratorType) {
  auto processVariable =
      _controlSystemPVManager->getProcessVariable(propertyDescription.source);

  // the DoocsProcessScalar needs the real ProcessScalar type, not just
  // ProcessVariable
  auto processArray =
      getDecorator<DOOCS_PRIMITIVE_T>(processVariable, decoratorType);

  assert(processArray->getNumberOfChannels() == 1);
  boost::shared_ptr<D_fct> doocsPV;
  // Histories seem to be supported by DOOCS only for property names shorter
  // than 64 characters, so disable history for longer names. The DOOCS property
  // name is the variable name without the location name and the separating
  // slash between location and property name. One has to subtract another 6
  // characters because Doocs automatically adds
  // "._HIST", which also has to fit into the 64 characters
  if (propertyDescription.name.length() > 64 - 6) {
    std::cerr << "WARNING: Disabling history for " << processArray->getName()
              << ". Name is too long." << std::endl;
    doocsPV.reset(new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(
        propertyDescription.name.c_str(), _eqFct, processArray, _updater));
  } else {
    if (propertyDescription.hasHistory) {
      // version with history: EqFtc first
      doocsPV.reset(new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(
          _eqFct, propertyDescription.name.c_str(), processArray, _updater));
    } else {
      // version without history: name first
      doocsPV.reset(new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(
          propertyDescription.name.c_str(), _eqFct, processArray, _updater));
    }
  } // if name too long

  // set read only mode if configured in the xml file or for output variables
  if (!processArray->isWriteable() || !propertyDescription.isWriteable) {
    doocsPV->set_ro_access();
  }

  // publish via ZeroMQ if configured in the xml file
  if (propertyDescription.publishZMQ) {
    boost::dynamic_pointer_cast<DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>>(
        doocsPV)
        ->publishZeroMQ();
  }

  // set macro pulse number source, if configured
  if (propertyDescription.macroPulseNumberSource.size() > 0) {
    auto mpnSource = _controlSystemPVManager->getProcessVariable(
        propertyDescription.macroPulseNumberSource);
    auto mpnDecorated =
        getDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
    if (mpnDecorated->getNumberOfSamples() != 1) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it has an array "
          "length of " +
          std::to_string(mpnDecorated->getNumberOfSamples()) +
          ". Length must be exactly 1");
    }
    if (!mpnDecorated->isReadable()) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it is not readable.");
    }
    boost::dynamic_pointer_cast<DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>>(
        doocsPV)
        ->setMacroPulseNumberSource(mpnDecorated);
    _updater.addVariable(
        ChimeraTK::ScalarRegisterAccessor<int64_t>(mpnDecorated), _eqFct,
        [] {});
  }

  return doocsPV;
}

template <>
boost::shared_ptr<D_fct>
DoocsPVFactory::createDoocsScalar<std::string, D_string>(
    AutoPropertyDescription const &propertyDescription,
    DecoratorType /*decoratorType*/) {
  auto processVariable =
      _controlSystemPVManager->getProcessVariable(propertyDescription.source);

  // FIXME: Use a decorator, but this has to be tested and implemented for
  // strings first the DoocsProcessArray needs the real ProcessScalar type, not
  // just ProcessVariable
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> processArray =
      boost::dynamic_pointer_cast<ChimeraTK::NDRegisterAccessor<std::string>>(
          processVariable);
  if (!processArray) {
    throw std::invalid_argument(
        std::string("DoocsPVFactory::createDoocsArray : processArray is of the "
                    "wrong type ") +
        processVariable->getValueType().name());
  }

  assert(processArray->getNumberOfChannels() == 1);
  assert(processArray->getNumberOfSamples() ==
         1); // array of strings is not supported
  boost::shared_ptr<D_fct> doocsPV(
      new DoocsProcessScalar<std::string, D_string>(
          _eqFct, propertyDescription.name.c_str(), processArray, _updater));

  // set read only mode if configures in the xml file or for output variables
  if (!processArray->isWriteable() || !propertyDescription.isWriteable) {
    doocsPV->set_ro_access();
  }

  // publish via ZeroMQ if configured in the xml file
  if (propertyDescription.publishZMQ) {
    boost::dynamic_pointer_cast<DoocsProcessScalar<std::string, D_string>>(
        doocsPV)
        ->publishZeroMQ();
  }

  // set macro pulse number source, if configured
  if (propertyDescription.macroPulseNumberSource.size() > 0) {
    auto mpnSource = _controlSystemPVManager->getProcessVariable(
        propertyDescription.macroPulseNumberSource);
    auto mpnDecorated =
        getDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
    if (mpnDecorated->getNumberOfSamples() != 1) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it has an array "
          "length of " +
          std::to_string(mpnDecorated->getNumberOfSamples()) +
          ". Length must be exactly 1");
    }
    if (!mpnDecorated->isReadable()) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it is not readable.");
    }
    boost::dynamic_pointer_cast<DoocsProcessScalar<std::string, D_string>>(
        doocsPV)
        ->setMacroPulseNumberSource(mpnDecorated);
    _updater.addVariable(
        ChimeraTK::ScalarRegisterAccessor<int64_t>(mpnDecorated), _eqFct,
        [] {});
  }

  return doocsPV;
}

boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsSpectrum(
    SpectrumDescription const &spectrumDescription) {
  auto processVariable =
      _controlSystemPVManager->getProcessVariable(spectrumDescription.source);
  float start = spectrumDescription.start;
  float increment = spectrumDescription.increment;

  // in case dynamic changing of the axis is requested replace the static values
  // from the config file with the data from the accessors. The spectrum will
  // keep the data updated.
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> startAccessor;
  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> incrementAccessor;

  if (spectrumDescription.startSource != "") {
    startAccessor =
        getDecorator<float>(_controlSystemPVManager->getProcessVariable(
                                spectrumDescription.startSource),
                            DecoratorType::C_style_conversion);
    start = startAccessor->accessData(0);
  }
  if (spectrumDescription.incrementSource != "") {
    incrementAccessor =
        getDecorator<float>(_controlSystemPVManager->getProcessVariable(
                                spectrumDescription.incrementSource),
                            DecoratorType::C_style_conversion);
    increment = incrementAccessor->accessData(0);
  }

  //    assert(processArray->getNumberOfChannels() == 1);
  boost::shared_ptr<D_fct> doocsPV(new DoocsSpectrum(
      _eqFct, spectrumDescription.name,
      getDecorator<float>(processVariable, DecoratorType::C_style_conversion),
      _updater, startAccessor, incrementAccessor));

  // set read only mode if configures in the xml file or for output variables
  if (!processVariable->isWriteable() || !spectrumDescription.isWriteable) {
    doocsPV->set_ro_access();
  }

  // can use static cast, we know it's a D_spectrum, we just created it
  auto spectrum = boost::static_pointer_cast<D_spectrum>(doocsPV);
  spectrum->spectrum_parameter(spectrum->spec_time(), start, increment,
                               spectrum->spec_status());

  // publish via ZeroMQ if configured in the xml file
  if (spectrumDescription.publishZMQ) {
    boost::dynamic_pointer_cast<DoocsSpectrum>(doocsPV)->publishZeroMQ();
  }

  // set macro pulse number source, if configured
  if (spectrumDescription.macroPulseNumberSource.size() > 0) {
    auto mpnSource = _controlSystemPVManager->getProcessVariable(
        spectrumDescription.macroPulseNumberSource);
    auto mpnDecorated =
        getDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
    if (mpnDecorated->getNumberOfSamples() != 1) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it has an array "
          "length of " +
          std::to_string(mpnDecorated->getNumberOfSamples()) +
          ". Length must be exactly 1");
    }
    if (!mpnDecorated->isReadable()) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it is not readable.");
    }
    boost::dynamic_pointer_cast<DoocsSpectrum>(doocsPV)
        ->setMacroPulseNumberSource(mpnDecorated);
  }

  return doocsPV;
}

// fixme: some of the variables needed here are redundant and can be sovled with
// mpl and/or fusion maps
template <class IMPL_T, class DOOCS_SCALAR_T, class DOOCS_PRIMITIVE_T,
          class DOOCS_ARRAY_T, class DOOCS_ARRAY_PRIMITIVE_T>
boost::shared_ptr<D_fct> DoocsPVFactory::typedCreateScalarOrArray(
    ProcessVariable &processVariable,
    AutoPropertyDescription const &autoPropertyDescription,
    DecoratorType decoratorType, ArrayDescription::DataType arrayDataType) {
  // We have to convert to the original NDRegisterAccessor to determine the
  // number of samples. We cannot use a decorator because scalar and array
  // DOOCS_PRIMITIVE_T can be different, and once a decorator is created you
  // cannot get the other type any more.

  auto &ndAccessor =
      dynamic_cast<ChimeraTK::NDRegisterAccessor<IMPL_T> &>(processVariable);
  auto nSamples = ndAccessor.getNumberOfSamples();

  if (nSamples == 1) {
    return createDoocsScalar<DOOCS_PRIMITIVE_T, DOOCS_SCALAR_T>(
        autoPropertyDescription, decoratorType);
  } else {
    return typedCreateDoocsArray<DOOCS_ARRAY_PRIMITIVE_T, DOOCS_ARRAY_T>(
        ArrayDescription(autoPropertyDescription, arrayDataType));
  }
}

boost::shared_ptr<D_fct> DoocsPVFactory::autoCreate(
    std::shared_ptr<PropertyDescription> const &propertyDescription) {
  // do auto creation
  auto autoPropertyDescription =
      std::static_pointer_cast<AutoPropertyDescription>(propertyDescription);

  auto pvName = autoPropertyDescription->source;
  auto processVariable = _controlSystemPVManager->getProcessVariable(pvName);

  std::type_info const &valueType = processVariable->getValueType();
  /*  TODO:
      - create functions "createDoocsArray" and "createDoocsSpectrum"
      - first use spectrum here for 1D, then switch to array (tests need to be
     adapted)
      - create spectrum, array and d_int/float/double upon request from 1d
     (scalar for D_array and 1D)
  */

  // fixme: make this a boost::foreach in the data types provided by
  // DeviceAccess. This will detect uncovered new data types at compile time,
  // not only at run time
  if (valueType == typeid(int8_t)) {
    return typedCreateScalarOrArray<int8_t, D_int, int32_t, D_bytearray,
                                    uint8_t>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Byte);
  } else if (valueType == typeid(uint8_t)) {
    return typedCreateScalarOrArray<uint8_t, D_int, int32_t, D_bytearray,
                                    uint8_t>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Byte);
  } else if (valueType == typeid(int16_t)) {
    return typedCreateScalarOrArray<int16_t, D_int, int32_t, D_shortarray,
                                    int16_t>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Short);
  } else if (valueType == typeid(uint16_t)) {
    return typedCreateScalarOrArray<uint16_t, D_int, int32_t, D_shortarray,
                                    int16_t>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Short);
  } else if (valueType == typeid(int32_t)) {
    return typedCreateScalarOrArray<int32_t, D_int, int32_t, D_intarray,
                                    int32_t>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Int);
  } else if (valueType == typeid(uint32_t)) {
    return typedCreateScalarOrArray<uint32_t, D_int, int32_t, D_intarray,
                                    int32_t>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Int);
  } else if (valueType == typeid(int64_t)) {
    // there is no scalar int64 representation in doocs, so we always create an
    // array, also for length = 1
    return typedCreateDoocsArray<int64_t, D_longarray>(ArrayDescription(
        *autoPropertyDescription, ArrayDescription::DataType::Long));
  } else if (valueType == typeid(uint64_t)) {
    return typedCreateDoocsArray<int64_t, D_longarray>(ArrayDescription(
        *autoPropertyDescription, ArrayDescription::DataType::Long));
  } else if (valueType == typeid(float)) {
    return typedCreateScalarOrArray<float, D_float, float, D_floatarray, float>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Float);
  } else if (valueType == typeid(double)) {
    return typedCreateScalarOrArray<double, D_double, double, D_doublearray,
                                    double>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::C_style_conversion, ArrayDescription::DataType::Double);
  } else if (valueType == typeid(std::string)) {
    return typedCreateScalarOrArray<std::string, D_string, std::string,
                                    std::nullptr_t, std::nullptr_t>(
        *processVariable, *autoPropertyDescription,
        DecoratorType::range_checking, ArrayDescription::DataType::Auto);
  } else {
    throw std::invalid_argument("unsupported value type");
  }
}

template <class DOOCS_PRIMITIVE_T, class DOOCS_T>
boost::shared_ptr<D_fct> DoocsPVFactory::typedCreateDoocsArray(
    ArrayDescription const &arrayDescription) {
  auto processVariable =
      _controlSystemPVManager->getProcessVariable(arrayDescription.source);

  ///@todo FIXME Add the decorator type as option  to the array description, and
  /// only use C_style_conversion as default
  boost::shared_ptr<D_fct> doocsPV(
      new DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>(
          _eqFct, arrayDescription.name,
          getDecorator<DOOCS_PRIMITIVE_T>(processVariable,
                                          DecoratorType::C_style_conversion),
          _updater));

  // set read only mode if configures in the xml file or for output variables
  if (!processVariable->isWriteable() || !arrayDescription.isWriteable) {
    doocsPV->set_ro_access();
  }

  // publish via ZeroMQ if configured in the xml file
  if (arrayDescription.publishZMQ) {
    boost::dynamic_pointer_cast<DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>>(
        doocsPV)
        ->publishZeroMQ();
  }

  // set macro pulse number source, if configured
  if (arrayDescription.macroPulseNumberSource.size() > 0) {
    auto mpnSource = _controlSystemPVManager->getProcessVariable(
        arrayDescription.macroPulseNumberSource);
    auto mpnDecorated =
        getDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
    if (mpnDecorated->getNumberOfSamples() != 1) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it has an array "
          "length of " +
          std::to_string(mpnDecorated->getNumberOfSamples()) +
          ". Length must be exactly 1");
    }
    if (!mpnDecorated->isReadable()) {
      throw ChimeraTK::logic_error(
          "The property '" + mpnDecorated->getName() +
          "' is used as a macro pulse number source, but it is not readable.");
    }
    boost::dynamic_pointer_cast<DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>>(
        doocsPV)
        ->setMacroPulseNumberSource(mpnDecorated);
    _updater.addVariable(
        ChimeraTK::ScalarRegisterAccessor<int64_t>(mpnDecorated), _eqFct,
        [] {});
  }

  return doocsPV;
}

// template specialisation for cases with no matching DOOCS array type (e.g.
// string)
template <>
boost::shared_ptr<D_fct>
DoocsPVFactory::typedCreateDoocsArray<std::nullptr_t, std::nullptr_t>(
    ArrayDescription const &) {
  throw std::invalid_argument("Type not supported as an array");
}

boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsArray(
    std::shared_ptr<ArrayDescription> const &arrayDescription) {
  if (arrayDescription->dataType == ArrayDescription::DataType::Auto) {
    // leave the desision which array to produce to the auto creation algorithm.
    // We need it there anyway
    // FIXME: This does not produce arrays of length 1 because it will produce a
    // scalar
    return autoCreate(arrayDescription);
  } else if (arrayDescription->dataType == ArrayDescription::DataType::Byte) {
    return typedCreateDoocsArray<uint8_t, D_bytearray>(*arrayDescription);
  } else if (arrayDescription->dataType == ArrayDescription::DataType::Short) {
    return typedCreateDoocsArray<int16_t, D_shortarray>(*arrayDescription);
  } else if (arrayDescription->dataType == ArrayDescription::DataType::Int) {
    return typedCreateDoocsArray<int32_t, D_intarray>(*arrayDescription);
  } else if (arrayDescription->dataType == ArrayDescription::DataType::Long) {
    return typedCreateDoocsArray<int64_t, D_longarray>(*arrayDescription);
  } else if (arrayDescription->dataType == ArrayDescription::DataType::Float) {
    return typedCreateDoocsArray<float, D_floatarray>(*arrayDescription);
  } else if (arrayDescription->dataType == ArrayDescription::DataType::Double) {
    return typedCreateDoocsArray<double, D_doublearray>(*arrayDescription);
  } else {
    throw std::logic_error(
        "DoocsPVFactory does not implement a data type it should!");
  }
}

boost::shared_ptr<D_fct> DoocsPVFactory::create(
    std::shared_ptr<PropertyDescription> const &propertyDescription) {
  auto &requestedType = propertyDescription->type();
  if (requestedType == typeid(AutoPropertyDescription)) {
    return autoCreate(propertyDescription);
  } else if (requestedType == typeid(SpectrumDescription)) {
    return createDoocsSpectrum(
        *std::static_pointer_cast<SpectrumDescription>(propertyDescription));
  } else if (requestedType == typeid(ArrayDescription)) {
    return createDoocsArray(
        std::static_pointer_cast<ArrayDescription>(propertyDescription));
  } else {
    throw std::invalid_argument("Sorry, your type is not supported yet.");
  }
}

} // namespace ChimeraTK
