#ifndef CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
#define CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H

#include <string>
//#include <map>
//#include <set>
//#include <memory>
//#include <iostream>
//#include <ChimeraTK/RegisterPath.h>

///@todo FIXME: Separate these out into individual files

namespace ChimeraTK {

  // PropertyAttributes are used in the property description itself, and
  // as default values (global and in the locations)
  struct PropertyAttributes {
    bool hasHistory;
    bool isWriteable;
    bool publishZMQ;
    PropertyAttributes(bool hasHistory_ = true, bool isWriteable_ = true, bool publishZMQ_ = false)
    : hasHistory(hasHistory_), isWriteable(isWriteable_), publishZMQ(publishZMQ_) {}
    bool operator==(PropertyAttributes const& other) const {
      return (hasHistory == other.hasHistory && isWriteable == other.isWriteable && publishZMQ == other.publishZMQ);
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
    ChimeraTK::RegisterPath source;
    AutoPropertyDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "",
        std::string name_ = "", bool hasHistory_ = true, bool isWriteable_ = true)
    : PropertyDescription(location_, name_), PropertyAttributes(hasHistory_, isWriteable_), source(source_) {}
    virtual bool operator==(PropertyDescription const& other) const override {
      if(other.type() == typeid(AutoPropertyDescription)) {
        auto casted_other = static_cast<AutoPropertyDescription const&>(other);
        return source == casted_other.source && location == other.location && name == other.name &&
            static_cast<const PropertyAttributes*>(this)->operator==(casted_other);
      }
      else {
        return false;
      }
    }
    virtual const std::type_info& type() const override { return typeid(AutoPropertyDescription); }
    virtual void print(std::ostream& os = std::cout) const {
      os << source << " -> " << location << " / " << name << std::endl;
    }
  };

  /********************************************************************************************************************/

  struct ArrayDescription : public AutoPropertyDescription {
    enum class DataType { Byte, Short, Int, Long, Float, Double, Auto };

    ArrayDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "", std::string name_ = "",
        DataType dataType_ = DataType::Auto, bool hasHistory_ = true, bool isWriteable_ = true)
    : AutoPropertyDescription(source_, location_, name_, hasHistory_, isWriteable_), dataType(dataType_) {}

    /// Convenience constructor from an auto property description, just add the type
    ArrayDescription(AutoPropertyDescription const& autoDescription, DataType dataType_)
    : AutoPropertyDescription(autoDescription), dataType(dataType_) {}

    virtual bool operator==(PropertyDescription const& other) const override {
      if(other.type() == typeid(ArrayDescription)) {
        auto casted_other = static_cast<ArrayDescription const&>(other);
        return dataType == casted_other.dataType &&
            static_cast<const AutoPropertyDescription*>(this)->operator==(casted_other);
      }
      else {
        return false;
      }
    }

    virtual const std::type_info& type() const { return typeid(ArrayDescription); }
    DataType dataType;
  };

  /********************************************************************************************************************/

  struct SpectrumDescription : public PropertyDescription, public PropertyAttributes {
    ChimeraTK::RegisterPath source;
    ChimeraTK::RegisterPath startSource;
    ChimeraTK::RegisterPath incrementSource;
    float start;
    float increment;

    SpectrumDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "", std::string name_ = "",
        bool hasHistory_ = true, bool isWriteable_ = true)
    : PropertyDescription(location_, name_), PropertyAttributes(hasHistory_, isWriteable_), source(source_), start(0),
      increment(1.0) {}

    virtual const std::type_info& type() const { return typeid(SpectrumDescription); }
    virtual void print(std::ostream& os = std::cout) const {
      os << source << " -> " << location << " / " << name << " (startSource = " << startSource
         << ", incrementSource = " << incrementSource << ")" << std::endl;
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

  // This is the per location information which are used as default for the properties in this
  // location
  struct LocationInfo : public PropertyAttributes {
    bool useHasHistoryDefault;
    bool useIsWriteableDefault;
    LocationInfo(bool useHasHistoryDefault_ = false, bool useIsWriteableDefault_ = false)
    : useHasHistoryDefault(useHasHistoryDefault_), useIsWriteableDefault(useIsWriteableDefault_) {}
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
