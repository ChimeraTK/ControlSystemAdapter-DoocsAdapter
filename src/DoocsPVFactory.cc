// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsPVFactory.h"

#include "D_textUnifier.h"
#include "DoocsIfff.h"
#include "DoocsIiii.h"
#include "DoocsImage.h"
#include "DoocsProcessArray.h"
#include "DoocsProcessScalar.h"
#include "DoocsSpectrum.h"
#include "DoocsXY.h"
#include "splitStringAtFirstSlash.h"
#include <d_fct.h>

#include <ChimeraTK/TypeChangingDecorator.h>

#include <boost/make_shared.hpp>

#include <utility>

namespace ChimeraTK {

  DoocsPVFactory::DoocsPVFactory(
      EqFct* const eqFct, DoocsUpdater& updater, boost::shared_ptr<ControlSystemPVManager> csPVManager)
  : _eqFct(eqFct), _updater(updater), _controlSystemPVManager(std::move(csPVManager)) {
    assert(eqFct != nullptr);
  }

  // Fixme: is AutoPropertyDescription ok, or to we need IntDescripton,
  // DoubleDescription etc.
  template<class DOOCS_PRIMITIVE_T, class DOOCS_T>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar(
      AutoPropertyDescription const& propertyDescription, DecoratorType decoratorType) {
    auto processVariable = _controlSystemPVManager->getProcessVariable(propertyDescription.source);

    // the DoocsProcessScalar needs the real ProcessScalar type, not just
    // ProcessVariable
    boost::shared_ptr<NDRegisterAccessor<DOOCS_PRIMITIVE_T>> processArray;
    if(typeid(DOOCS_PRIMITIVE_T) == processVariable->getValueType()) {
      processArray = boost::dynamic_pointer_cast<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>>(processVariable);
    }
    else {
      processArray = getTypeChangingDecorator<DOOCS_PRIMITIVE_T>(processVariable, decoratorType);
    }

    assert(processArray->getNumberOfChannels() == 1);
    boost::shared_ptr<D_fct> doocsPV;
    // Histories seem to be supported by DOOCS only for property names shorter
    // than 64 characters, so disable history for longer names. The DOOCS property
    // name is the variable name without the location name and the separating
    // slash between location and property name. One has to subtract another 6
    // characters because Doocs automatically adds
    // "._HIST", which also has to fit into the 64 characters
    if(propertyDescription.name.length() > 64 - 6) {
      std::cerr << "WARNING: Disabling history for " << processArray->getName() << ". Name is too long." << std::endl;
      doocsPV.reset(
          new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(propertyDescription.name, _eqFct, processArray, _updater));
    }
    else {
      if(propertyDescription.hasHistory) {
        // version with history: EqFtc first
        doocsPV.reset(new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(
            _eqFct, propertyDescription.name, processArray, _updater));
      }
      else {
        // version without history: name first
        doocsPV.reset(new DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>(
            propertyDescription.name, _eqFct, processArray, _updater));
      }
    } // if name too long

    // set read only mode if configured in the xml file or for output variables
    if(!processArray->isWriteable() || !propertyDescription.isWriteable) {
      doocsPV->set_ro_access();
    }

    // publish via ZeroMQ if configured in the xml file
    if(propertyDescription.publishZMQ) {
      boost::dynamic_pointer_cast<DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>>(doocsPV)->publishZeroMQ();
    }

    // set data matching mode (need to call before setMacroPulseNumberSource, as the mode is checked there)
    boost::dynamic_pointer_cast<DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>>(doocsPV)->setMatchingMode(
        propertyDescription.dataMatching);

    // set macro pulse number source, if configured
    if(!propertyDescription.macroPulseNumberSource.empty()) {
      auto mpnSource = _controlSystemPVManager->getProcessVariable(propertyDescription.macroPulseNumberSource);
      auto mpnDecorated = getTypeChangingDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
      if(mpnDecorated->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnDecorated->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnDecorated->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      boost::dynamic_pointer_cast<DoocsProcessScalar<DOOCS_PRIMITIVE_T, DOOCS_T>>(doocsPV)->setMacroPulseNumberSource(
          mpnDecorated);
    }

    return doocsPV;
  }

  template<>
  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar<std::string, DTextUnifier>(
      AutoPropertyDescription const& propertyDescription, DecoratorType /*decoratorType*/) {
    auto processVariable = _controlSystemPVManager->getProcessVariable(propertyDescription.source);

    // FIXME: Use a decorator, but this has to be tested and implemented for
    // strings first the DoocsProcessArray needs the real ProcessScalar type, not
    // just ProcessVariable
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<std::string>> processArray =
        boost::dynamic_pointer_cast<ChimeraTK::NDRegisterAccessor<std::string>>(processVariable);
    if(!processArray) {
      throw std::invalid_argument(std::string("DoocsPVFactory::createDoocsArray : processArray is of the "
                                              "wrong type ") +
          processVariable->getValueType().name());
    }

    assert(processArray->getNumberOfChannels() == 1);
    assert(processArray->getNumberOfSamples() == 1); // array of strings is not supported
    boost::shared_ptr<DoocsProcessScalar<std::string, DTextUnifier>> doocsPV(
        new DoocsProcessScalar<std::string, DTextUnifier>(_eqFct, propertyDescription.name, processArray, _updater));

    // set read only mode if configures in the xml file or for output variables
    if(!processArray->isWriteable() || !propertyDescription.isWriteable) {
      doocsPV->set_ro_access();
    }

    // publish via ZeroMQ if configured in the xml file
    if(propertyDescription.publishZMQ) {
      doocsPV->publishZeroMQ();
    }

    // set data matching mode (need to call before setMacroPulseNumberSource, as the mode is checked there)
    doocsPV->setMatchingMode(propertyDescription.dataMatching);

    // set macro pulse number source, if configured
    if(!propertyDescription.macroPulseNumberSource.empty()) {
      auto mpnSource = _controlSystemPVManager->getProcessVariable(propertyDescription.macroPulseNumberSource);
      auto mpnDecorated = getTypeChangingDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
      if(mpnDecorated->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnDecorated->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnDecorated->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      doocsPV->setMacroPulseNumberSource(mpnDecorated);
    }

    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsSpectrum(SpectrumDescription const& spectrumDescription) {
    auto processVariable = _controlSystemPVManager->getProcessVariable(spectrumDescription.source);

    float start = spectrumDescription.start;
    float increment = spectrumDescription.increment;

    // in case dynamic changing of the axis is requested replace the static values
    // from the config file with the data from the accessors. The spectrum will
    // keep the data updated.
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> startAccessor;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> incrementAccessor;

    if(spectrumDescription.startSource != "") {
      startAccessor =
          getTypeChangingDecorator<float>(_controlSystemPVManager->getProcessVariable(spectrumDescription.startSource),
              DecoratorType::C_style_conversion);
      start = startAccessor->accessData(0);
    }
    if(spectrumDescription.incrementSource != "") {
      incrementAccessor = getTypeChangingDecorator<float>(
          _controlSystemPVManager->getProcessVariable(spectrumDescription.incrementSource),
          DecoratorType::C_style_conversion);
      increment = incrementAccessor->accessData(0);
    }

    //    assert(processArray->getNumberOfChannels() == 1);
    boost::shared_ptr<DoocsSpectrum> doocsPV;
    if(spectrumDescription.numberOfBuffers == 1) {
      doocsPV.reset(new DoocsSpectrum(_eqFct, spectrumDescription.name,
          getTypeChangingDecorator<float>(processVariable, DecoratorType::C_style_conversion), _updater, startAccessor,
          incrementAccessor));
    }
    else {
      doocsPV.reset(new DoocsSpectrum(_eqFct, spectrumDescription.name,
          getTypeChangingDecorator<float>(processVariable, DecoratorType::C_style_conversion), _updater, startAccessor,
          incrementAccessor, spectrumDescription.numberOfBuffers));
    }

    // set read only mode if configures in the xml file or for output variables
    if(!processVariable->isWriteable() || !spectrumDescription.isWriteable) {
      doocsPV->set_ro_access();
    }

    // can use static cast, we know it's a D_spectrum, we just created it
    auto spectrum = boost::static_pointer_cast<D_spectrum>(doocsPV);
    spectrum->spectrum_parameter(spectrum->spec_time(), start, increment, spectrum->spec_status());

    // publish via ZeroMQ if configured in the xml file
    if(spectrumDescription.publishZMQ) {
      boost::dynamic_pointer_cast<DoocsSpectrum>(doocsPV)->publishZeroMQ();
    }

    if(not spectrumDescription.description.empty()) {
      spectrum->description(spectrumDescription.description);
    }

    auto const xIt = spectrumDescription.axis.find("x");
    if(xIt != spectrumDescription.axis.cend()) {
      auto const& axis = xIt->second;
      spectrum->xegu(axis.logarithmic, axis.start, axis.stop, axis.label.c_str());
    }

    auto const yIt = spectrumDescription.axis.find("y");
    if(yIt != spectrumDescription.axis.cend()) {
      auto const& axis = yIt->second;
      spectrum->egu(axis.logarithmic, axis.start, axis.stop, axis.label.c_str());
    }

    // set data matching mode (need to call before setMacroPulseNumberSource, as the mode is checked there)
    doocsPV->setMatchingMode(spectrumDescription.dataMatching);

    // set macro pulse number source, if configured
    if(!spectrumDescription.macroPulseNumberSource.empty()) {
      auto mpnSource = _controlSystemPVManager->getProcessVariable(spectrumDescription.macroPulseNumberSource);
      auto mpnDecorated = getTypeChangingDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
      if(mpnDecorated->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnDecorated->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnDecorated->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      doocsPV->setMacroPulseNumberSource(mpnDecorated);
    }
    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsImage(ImageDescription const& imageDescription) {
    auto processVariable = _controlSystemPVManager->getProcessVariable(imageDescription.source);
    boost::shared_ptr<DoocsImage> doocsPV = boost::make_shared<DoocsImage>(_eqFct, imageDescription.name,
        getTypeChangingDecorator<unsigned char>(processVariable, DecoratorType::C_style_conversion), _updater);

    if(not imageDescription.description.empty()) {
      doocsPV->set_descr_value(imageDescription.description);
      // D_image: doocsPV->set_img_comment(imageDescription.description.c_str());
    }
    doocsPV->set_ro_access();

    // publish via ZeroMQ if configured in the xml file
    if(imageDescription.publishZMQ) {
      doocsPV->publishZeroMQ();
    }

    // set data matching mode (need to call before setMacroPulseNumberSource, as the mode is checked there)
    doocsPV->setMatchingMode(imageDescription.dataMatching);
    // set macro pulse number source, if configured
    if(!imageDescription.macroPulseNumberSource.empty()) {
      auto mpnSource = _controlSystemPVManager->getProcessVariable(imageDescription.macroPulseNumberSource);
      auto mpnDecorated = getTypeChangingDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
      if(mpnDecorated->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnDecorated->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnDecorated->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      doocsPV->setMacroPulseNumberSource(mpnDecorated);
    }
    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::createXy(XyDescription const& xyDescription) {
    auto xProcessVariable = _controlSystemPVManager->getProcessVariable(xyDescription.xSource);
    auto yProcessVariable = _controlSystemPVManager->getProcessVariable(xyDescription.ySource);

    boost::shared_ptr<DoocsXy> doocsPV;
    doocsPV.reset(new DoocsXy(_eqFct, xyDescription.name,
        getTypeChangingDecorator<float>(xProcessVariable, DecoratorType::C_style_conversion),
        getTypeChangingDecorator<float>(yProcessVariable, DecoratorType::C_style_conversion), _updater));

    auto xy = boost::static_pointer_cast<DoocsXy>(doocsPV);

    if(not xyDescription.description.empty()) {
      xy->description(xyDescription.description);
    }

    auto const xIt = xyDescription.axis.find("x");
    if(xIt != xyDescription.axis.cend()) {
      auto const& axis = xIt->second;
      xy->xegu(axis.logarithmic, axis.start, axis.stop, axis.label.c_str());
    }

    auto const yIt = xyDescription.axis.find("y");
    if(yIt != xyDescription.axis.cend()) {
      auto const& axis = yIt->second;
      xy->egu(axis.logarithmic, axis.start, axis.stop, axis.label.c_str());
    }

    if(xyDescription.publishZMQ) {
      doocsPV->publishZeroMQ();
    }

    doocsPV->set_ro_access();

    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::createIfff(IfffDescription const& ifffDescription) {
    auto i1ProcessVariable = _controlSystemPVManager->getProcessVariable(ifffDescription.i1Source);
    auto f1ProcessVariable = _controlSystemPVManager->getProcessVariable(ifffDescription.f1Source);
    auto f2ProcessVariable = _controlSystemPVManager->getProcessVariable(ifffDescription.f2Source);
    auto f3ProcessVariable = _controlSystemPVManager->getProcessVariable(ifffDescription.f3Source);

    boost::shared_ptr<DoocsIfff> doocsPV;
    doocsPV.reset(new DoocsIfff(_eqFct, ifffDescription.name,
        getTypeChangingDecorator<int>(i1ProcessVariable, DecoratorType::C_style_conversion),
        getTypeChangingDecorator<float>(f1ProcessVariable, DecoratorType::C_style_conversion),
        getTypeChangingDecorator<float>(f2ProcessVariable, DecoratorType::C_style_conversion),
        getTypeChangingDecorator<float>(f3ProcessVariable, DecoratorType::C_style_conversion), _updater));

    // set specified data_matching mode
    doocsPV->setMatchingMode(ifffDescription.dataMatching);

    // set macro pulse number source, if configured
    if(!ifffDescription.macroPulseNumberSource.empty()) {
      auto mpnSource = _controlSystemPVManager->getProcessVariable(ifffDescription.macroPulseNumberSource);
      auto mpnDecorated = getTypeChangingDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
      if(mpnDecorated->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnDecorated->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnDecorated->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      doocsPV->setMacroPulseNumberSource(mpnDecorated);
    }

    if(ifffDescription.publishZMQ) {
      doocsPV->publishZeroMQ();
    }

    if(not ifffDescription.isWriteable) {
      doocsPV->set_ro_access();
    }
    return doocsPV;
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::createIiii(IiiiDescription const& iiiiDescription) {
    auto iiiiProcessVariable = _controlSystemPVManager->getProcessVariable(iiiiDescription.iiiiSource);

    boost::shared_ptr<DoocsIiii> doocsPV;
    doocsPV.reset(new DoocsIiii(_eqFct, iiiiDescription.name,
        getTypeChangingDecorator<int>(iiiiProcessVariable, DecoratorType::C_style_conversion), _updater));

    // set specified data_matching mode
    doocsPV->setMatchingMode(iiiiDescription.dataMatching);

    // set macro pulse number source, if configured
    if(iiiiDescription.macroPulseNumberSource.size() > 0) {
      auto mpnSource = _controlSystemPVManager->getProcessVariable(iiiiDescription.macroPulseNumberSource);
      auto mpnDecorated = getTypeChangingDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
      if(mpnDecorated->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnDecorated->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnDecorated->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      doocsPV->setMacroPulseNumberSource(mpnDecorated);
    }

    if(iiiiDescription.publishZMQ) {
      doocsPV->publishZeroMQ();
    }

    if(not iiiiDescription.isWriteable) {
      doocsPV->set_ro_access();
    }
    return doocsPV;
  }

  // fixme: some of the variables needed here are redundant and can be sovled with
  // mpl and/or fusion maps
  template<class DOOCS_SCALAR_T, class DOOCS_PRIMITIVE_T, class DOOCS_ARRAY_T, class DOOCS_ARRAY_PRIMITIVE_T>
  boost::shared_ptr<D_fct> DoocsPVFactory::typedCreateScalarOrArray(const std::type_info& valueType,
      ProcessVariable& processVariable, AutoPropertyDescription const& autoPropertyDescription,
      DecoratorType decoratorType) {
    // We have to convert to the original NDRegisterAccessor to determine the
    // number of samples. We cannot use a decorator because scalar and array
    // DOOCS_PRIMITIVE_T can be different, and once a decorator is created you
    // cannot get the other type any more.

    size_t nSamples;
    callForType(valueType, [&](auto t) {
      using T = decltype(t);
      nSamples = dynamic_cast<ChimeraTK::NDRegisterAccessor<T>&>(processVariable).getNumberOfSamples();
    });

    if(nSamples == 1) {
      return createDoocsScalar<DOOCS_PRIMITIVE_T, DOOCS_SCALAR_T>(autoPropertyDescription, decoratorType);
    }
    return typedCreateDoocsArray<DOOCS_ARRAY_PRIMITIVE_T, DOOCS_ARRAY_T>(
        AutoPropertyDescription(autoPropertyDescription));
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::autoCreate(std::shared_ptr<PropertyDescription> const& propertyDescription) {
    // do auto creation
    auto autoPropertyDescription = std::static_pointer_cast<AutoPropertyDescription>(propertyDescription);

    auto pvName = autoPropertyDescription->source;
    auto processVariable = _controlSystemPVManager->getProcessVariable(pvName);

    std::type_info const& valueType = processVariable->getValueType();
    /*  TODO:
        - create functions "createDoocsArray" and "createDoocsSpectrum"
        - first use spectrum here for 1D, then switch to array (tests need to be
       adapted)
        - create spectrum, array and d_int/float/double upon request from 1d
       (scalar for D_array and 1D)
    */

    if(autoPropertyDescription->dataType == AutoPropertyDescription::DataType::Auto) {
      autoPropertyDescription->deriveType(valueType);
    }

    switch(autoPropertyDescription->dataType) {
      case AutoPropertyDescription::DataType::Byte:
        return typedCreateScalarOrArray<D_int, int32_t, D_bytearray, uint8_t>(
            valueType, *processVariable, *autoPropertyDescription, DecoratorType::C_style_conversion);
      case AutoPropertyDescription::DataType::Short:
        return typedCreateScalarOrArray<D_int, int32_t, D_shortarray, int16_t>(
            valueType, *processVariable, *autoPropertyDescription, DecoratorType::C_style_conversion);
      case AutoPropertyDescription::DataType::Int:
        return typedCreateScalarOrArray<D_int, int32_t, D_intarray, int32_t>(
            valueType, *processVariable, *autoPropertyDescription, DecoratorType::C_style_conversion);
      case AutoPropertyDescription::DataType::Long:
        return typedCreateDoocsArray<int64_t, D_longarray>(AutoPropertyDescription(*autoPropertyDescription));
      case AutoPropertyDescription::DataType::Float:
        return typedCreateScalarOrArray<D_float, float, D_floatarray, float>(
            valueType, *processVariable, *autoPropertyDescription, DecoratorType::C_style_conversion);
      case AutoPropertyDescription::DataType::Double:
        return typedCreateScalarOrArray<D_double, double, D_doublearray, double>(
            valueType, *processVariable, *autoPropertyDescription, DecoratorType::C_style_conversion);
      case AutoPropertyDescription::DataType::Bool:
        return typedCreateScalarOrArray<doocs::D_value<bool>, ChimeraTK::Boolean, doocs::D_array<int32_t>, int32_t>(
            valueType, *processVariable, *autoPropertyDescription, DecoratorType::C_style_conversion);
      case AutoPropertyDescription::DataType::Void:
        // TODO: Map this to something more appropriate, e.g. D_fwd
        return typedCreateScalarOrArray<D_int, int32_t, D_intarray, int32_t>(
            valueType, *processVariable, *autoPropertyDescription, DecoratorType::C_style_conversion);
      case AutoPropertyDescription::DataType::Auto:
        if(valueType == typeid(std::string)) {
          return typedCreateScalarOrArray<DTextUnifier, std::string, std::nullptr_t, std::nullptr_t>(
              valueType, *processVariable, *autoPropertyDescription, DecoratorType::limiting);
        }
        throw std::logic_error("DoocsPVFactory does not implement a data type it should!");
    }

    // Make compiler happy
    throw std::logic_error("Should not be reached");
  }

  template<class DOOCS_PRIMITIVE_T, class DOOCS_T>
  boost::shared_ptr<D_fct> DoocsPVFactory::typedCreateDoocsArray(AutoPropertyDescription const& propertyDescription) {
    auto processVariable = _controlSystemPVManager->getProcessVariable(propertyDescription.source);

    // the DoocsProcessScalar needs the real ProcessScalar type, not just
    // ProcessVariable
    boost::shared_ptr<NDRegisterAccessor<DOOCS_PRIMITIVE_T>> processArray;
    if(typeid(DOOCS_PRIMITIVE_T) == processVariable->getValueType()) {
      processArray = boost::dynamic_pointer_cast<ChimeraTK::NDRegisterAccessor<DOOCS_PRIMITIVE_T>>(processVariable);
    }
    else {
      processArray = getTypeChangingDecorator<DOOCS_PRIMITIVE_T>(processVariable, DecoratorType::C_style_conversion);
    }

    ///@todo FIXME Add the decorator type as option  to the array description, and
    /// only use C_style_conversion as default
    boost::shared_ptr<PropertyBase> doocsPV(
        new DoocsProcessArray<DOOCS_T, DOOCS_PRIMITIVE_T>(_eqFct, propertyDescription.name, processArray, _updater));

    // set read only mode if configures in the xml file or for output variables
    if(!processVariable->isWriteable() || !propertyDescription.isWriteable) {
      doocsPV->getDfct()->set_ro_access();
    }

    // publish via ZeroMQ if configured in the xml file
    if(propertyDescription.publishZMQ) {
      doocsPV->publishZeroMQ();
    }

    // set data matching mode (need to call before setMacroPulseNumberSource, as the mode is checked there)
    doocsPV->setMatchingMode(propertyDescription.dataMatching);

    // set macro pulse number source, if configured
    if(!propertyDescription.macroPulseNumberSource.empty()) {
      auto mpnSource = _controlSystemPVManager->getProcessVariable(propertyDescription.macroPulseNumberSource);
      auto mpnDecorated = getTypeChangingDecorator<int64_t>(mpnSource, DecoratorType::C_style_conversion);
      if(mpnDecorated->getNumberOfSamples() != 1) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it has an array length of " +
            std::to_string(mpnDecorated->getNumberOfSamples()) + ". Length must be exactly 1");
      }
      if(!mpnDecorated->isReadable()) {
        throw ChimeraTK::logic_error("The property '" + mpnDecorated->getName() +
            "' is used as a macro pulse number source, but it is not readable.");
      }
      doocsPV->setMacroPulseNumberSource(mpnDecorated);
    }

    return boost::dynamic_pointer_cast<D_fct>(doocsPV);
  }

  // template specialisation for cases with no matching DOOCS array type (e.g.
  // string)
  template<>
  boost::shared_ptr<D_fct> DoocsPVFactory::typedCreateDoocsArray<std::nullptr_t, std::nullptr_t>(
      AutoPropertyDescription const&) {
    throw std::invalid_argument("Type not supported as an array");
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsArray(
      std::shared_ptr<AutoPropertyDescription> const& propertyDescription) {
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Auto) {
      // leave the decision which array to produce to the auto creation algorithm.
      // We need it there anyway
      // FIXME: This does not produce arrays of length 1 because it will produce a
      // scalar
      return autoCreate(propertyDescription);
    }
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Byte) {
      return typedCreateDoocsArray<uint8_t, D_bytearray>(*propertyDescription);
    }
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Short) {
      return typedCreateDoocsArray<int16_t, D_shortarray>(*propertyDescription);
    }
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Int) {
      return typedCreateDoocsArray<int32_t, D_intarray>(*propertyDescription);
    }
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Long) {
      return typedCreateDoocsArray<int64_t, D_longarray>(*propertyDescription);
    }
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Float) {
      return typedCreateDoocsArray<float, D_floatarray>(*propertyDescription);
    }
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Double) {
      return typedCreateDoocsArray<double, D_doublearray>(*propertyDescription);
    }
    if(propertyDescription->dataType == AutoPropertyDescription::DataType::Bool) {
      return typedCreateDoocsArray<int32_t, doocs::D_array<int32_t>>(*propertyDescription);
    }
    throw std::logic_error("DoocsPVFactory does not implement a data type it should!");
  }

  boost::shared_ptr<D_fct> DoocsPVFactory::create(std::shared_ptr<PropertyDescription> const& propertyDescription) {
    auto& plainDescription = *propertyDescription;
    const auto& requestedType = typeid(plainDescription);
    if(requestedType == typeid(AutoPropertyDescription)) {
      return autoCreate(propertyDescription);
    }
    if(requestedType == typeid(SpectrumDescription)) {
      return createDoocsSpectrum(*std::static_pointer_cast<SpectrumDescription>(propertyDescription));
    }
    if(requestedType == typeid(ImageDescription)) {
      return createDoocsImage(*std::static_pointer_cast<ImageDescription>(propertyDescription));
    }
    if(requestedType == typeid(XyDescription)) {
      return createXy(*std::static_pointer_cast<XyDescription>(propertyDescription));
    }
    if(requestedType == typeid(IfffDescription)) {
      return createIfff(*std::static_pointer_cast<IfffDescription>(propertyDescription));
    }
    if(requestedType == typeid(IiiiDescription)) {
      return createIiii(*std::static_pointer_cast<IiiiDescription>(propertyDescription));
    }
    if(requestedType == typeid(AutoPropertyDescription)) {
      return createDoocsArray(std::static_pointer_cast<AutoPropertyDescription>(propertyDescription));
    }
    throw std::invalid_argument("Sorry, your type is not supported yet.");
  }

} // namespace ChimeraTK
