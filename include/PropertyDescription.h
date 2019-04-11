#ifndef CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
#define CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H

#include <string>
//#include <map>
//#include <set>
//#include <memory>
//#include <iostream>
#include <ChimeraTK/RegisterPath.h>

///@todo FIXME: Separate these out into individual files

namespace ChimeraTK {

  // PropertyAttributes are used in the property description itself, and
  // as default values (global and in the locations)
  struct PropertyAttributes {
    bool hasHistory;
    bool isWriteable;
    bool publishZMQ;
    std::string macroPulseNumberSource;
    PropertyAttributes(bool hasHistory_ = true, bool isWriteable_ = true, bool publishZMQ_ = false,
        std::string macroPulseNumberSource_ = "")
    : hasHistory(hasHistory_), isWriteable(isWriteable_), publishZMQ(publishZMQ_),
      macroPulseNumberSource(macroPulseNumberSource_) {}
    bool operator==(PropertyAttributes const& other) const {
      return (hasHistory == other.hasHistory && isWriteable == other.isWriteable && publishZMQ == other.publishZMQ &&
          macroPulseNumberSource == other.macroPulseNumberSource);
    }
  };

  /********************************************************************************************************************/

  // Common for all properties, the base class to be stored.
  // FIXME: should sort by name to put it into a set?
  struct PropertyDescription {
    std::string location;
    std::string name;
    PropertyDescription(std::string location_ = "", std::string name_ = "") : location(location_), name(name_) {}
    virtual bool operator==(PropertyDescription const& other) const {
      return location == other.location && name == other.name;
    }
    virtual const std::type_info& type() const { return typeid(PropertyDescription); }
    virtual void print(std::ostream& os = std::cout) const { os << location << " / " << name << std::endl; }
  };

  /********************************************************************************************************************/

  // Combines property attributes and the base description
  // FIXME: should sort by name to put it into a set?
  struct AutoPropertyDescription : public PropertyDescription, public PropertyAttributes {
    enum class DataType { Byte, Short, Int, Long, Float, Double, Auto };
    ChimeraTK::RegisterPath source;
    AutoPropertyDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "",
        std::string name_ = "", DataType dataType_ = DataType::Auto, bool hasHistory_ = true, bool isWriteable_ = true)
    : PropertyDescription(location_, name_), PropertyAttributes(hasHistory_, isWriteable_), source(source_),
      dataType(dataType_) {}
    virtual bool operator==(PropertyDescription const& other) const override {
      if(other.type() == typeid(AutoPropertyDescription)) {
        auto casted_other = static_cast<AutoPropertyDescription const&>(other);
        return dataType == casted_other.dataType && source == casted_other.source && location == other.location &&
            name == other.name && static_cast<const PropertyAttributes*>(this)->operator==(casted_other);
      }
      else {
        return false;
      }
    }
    virtual const std::type_info& type() const override { return typeid(AutoPropertyDescription); }
    virtual void print(std::ostream& os = std::cout) const {
      os << source << " -> " << location << " / " << name << std::endl;
    }

    void deriveType(std::type_info const& info) {
      if(info == typeid(uint8_t) || info == typeid(int8_t)) dataType = DataType::Byte;
      if(info == typeid(uint16_t) || info == typeid(int16_t)) dataType = DataType::Short;
      if(info == typeid(uint32_t) || info == typeid(int32_t)) dataType = DataType::Int;
      if(info == typeid(uint64_t) || info == typeid(int64_t)) dataType = DataType::Long;
      if(info == typeid(float)) dataType = DataType::Float;
      if(info == typeid(double)) dataType = DataType::Double;
    }

    DataType dataType;
  };

  /********************************************************************************************************************/

  struct SpectrumDescription : public PropertyDescription, public PropertyAttributes {
    ChimeraTK::RegisterPath source;
    ChimeraTK::RegisterPath startSource;
    ChimeraTK::RegisterPath incrementSource;
    float start;
    float increment;
    size_t numberOfBuffers;

    SpectrumDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "", std::string name_ = "",
        bool hasHistory_ = true, bool isWriteable_ = true)
    : PropertyDescription(location_, name_), PropertyAttributes(hasHistory_, isWriteable_), source(source_), start(0),
      increment(1.0), numberOfBuffers(1) {}

    virtual const std::type_info& type() const { return typeid(SpectrumDescription); }
    virtual void print(std::ostream& os = std::cout) const {
      os << source << " -> " << location << " / " << name << " (startSource = " << startSource
         << ", incrementSource = " << incrementSource << ", numberOfBuffers = " << numberOfBuffers << ")" << std::endl;
    }
  };

  //    struct SpectrumDescription:
  //      public PropertyDescription, public PropertyAttributes{
  //      float xStart;
  //      float xIncrement;
  //      ChimeraTK::RegisterPath mainSource;
  //      ChimeraTK::RegisterPath xStartSource;
  //      ChimeraTK::RegisterPath xIncrementSource;
  //      SpectrumDescription(float start_=0.0, float increment_=1.0)
  //      : start(start_), increment(increment_){}
  //      bool operator==(SpectrumDescription const & other) const{
  //        return start==other.start && increment==other.increment;
  //      }
  //    };

  /********************************************************************************************************************/

  // This is the per location information which are used as default for the
  // properties in this location
  struct LocationInfo : public PropertyAttributes {
    bool useHasHistoryDefault;
    bool useIsWriteableDefault;
    bool useMacroPulseNumberSourceDefault;
    LocationInfo(bool useHasHistoryDefault_ = false, bool useIsWriteableDefault_ = false,
        bool useMacroPulseNumberSourceDefault_ = false)
    : useHasHistoryDefault(useHasHistoryDefault_), useIsWriteableDefault(useIsWriteableDefault_),
      useMacroPulseNumberSourceDefault(useMacroPulseNumberSourceDefault_) {}
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
