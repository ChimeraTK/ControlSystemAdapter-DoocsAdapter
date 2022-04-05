#ifndef CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
#define CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H

#include <string>
//#include <map>
//#include <set>
//#include <memory>
//#include <iostream>
#include <ChimeraTK/RegisterPath.h>
#include <ChimeraTK/DataConsistencyGroup.h>

///@todo FIXME: Separate these out into individual files

namespace ChimeraTK {

  /// configuration choices for writing persistance files for DOOCS properties
  struct PersistConfig {
    enum {
      OFF = 0,
      ON = 1,  // write long arrays to separate files below hist/, short arrays to config file
      AUTO = 2 // default DOOCS behaviour: persist only short arrays, in config file
    };
    int val;

    PersistConfig(std::string txt) {
      transform(txt.begin(), txt.end(), txt.begin(), ::tolower);
      if(txt == "false" or txt =="off" or txt == "0") {
        val = OFF;
      }
      else if(txt == "true" or txt == "on" or txt == "1") {
        val = ON;
      }
      else if(txt == "auto") {
        val = AUTO;
      }
      else {
        throw std::invalid_argument(std::string("Error parsing xml file: invalid input for PersistConfig: ") + txt);
      }
    }
    PersistConfig(int enumVal) {
      this->val = enumVal;
    }
    bool operator==(const PersistConfig& other) const { return val==other.val;}
  };

  // PropertyAttributes are used in the property description itself, and
  // as default values (global and in the locations)
  struct PropertyAttributes {
    bool hasHistory;
    bool isWriteable;
    bool publishZMQ;
    std::string macroPulseNumberSource;
    DataConsistencyGroup::MatchingMode dataMatching;
    PersistConfig persist = PersistConfig::ON;
    PropertyAttributes(bool hasHistory_ = true, bool isWriteable_ = true, bool publishZMQ_ = false,
        std::string macroPulseNumberSource_ = "",
        DataConsistencyGroup::MatchingMode dataMatching_ = DataConsistencyGroup::MatchingMode::exact)
    : hasHistory(hasHistory_), isWriteable(isWriteable_), publishZMQ(publishZMQ_),
      macroPulseNumberSource(macroPulseNumberSource_), dataMatching(dataMatching_) {}
    bool operator==(PropertyAttributes const& other) const {
      return (hasHistory == other.hasHistory && isWriteable == other.isWriteable && publishZMQ == other.publishZMQ &&
              macroPulseNumberSource == other.macroPulseNumberSource && dataMatching == other.dataMatching &&
              persist == other.persist);
    }
  };

  /********************************************************************************************************************/

  // Common for all properties, the base class to be stored.
  // FIXME: should sort by name to put it into a set?
  struct PropertyDescription {
    std::string location;
    std::string name;
    PropertyDescription(std::string location_ = "", std::string name_ = "") : location(location_), name(name_) {}
    virtual ~PropertyDescription() {}
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
    enum class DataType { Byte, Short, Int, Long, Float, Double, Bool, Void, Auto };
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
    const std::type_info& type() const override { return typeid(AutoPropertyDescription); }
    void print(std::ostream& os = std::cout) const override {
      os << source << " -> " << location << " / " << name << std::endl;
    }

    void deriveType(std::type_info const& info) {
      if(info == typeid(uint8_t) || info == typeid(int8_t)) dataType = DataType::Byte;
      if(info == typeid(uint16_t) || info == typeid(int16_t)) dataType = DataType::Short;
      if(info == typeid(uint32_t) || info == typeid(int32_t)) dataType = DataType::Int;
      if(info == typeid(uint64_t) || info == typeid(int64_t)) dataType = DataType::Long;
      if(info == typeid(float)) dataType = DataType::Float;
      if(info == typeid(double)) dataType = DataType::Double;
      if(info == typeid(ChimeraTK::Boolean)) dataType = DataType::Bool;
      if(info == typeid(ChimeraTK::Void)) dataType = DataType::Void;
    }

    DataType dataType;
  };

  /********************************************************************************************************************/

  struct SpectrumDescription : public PropertyDescription, public PropertyAttributes {
    struct Axis {
      std::string label;
      int logarithmic;
      float start;
      float stop;
    };

    ChimeraTK::RegisterPath source;
    ChimeraTK::RegisterPath startSource;
    ChimeraTK::RegisterPath incrementSource;
    float start;
    float increment;
    size_t numberOfBuffers;
    std::string description;
    std::map<std::string, Axis> axis;

    SpectrumDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "", std::string name_ = "",
        bool hasHistory_ = true, bool isWriteable_ = true)
    : PropertyDescription(location_, name_), PropertyAttributes(hasHistory_, isWriteable_), source(source_), start(0),
      increment(1.0), numberOfBuffers(1) {}

    const std::type_info& type() const override { return typeid(SpectrumDescription); }
    void print(std::ostream& os = std::cout) const override {
      os << source << " -> " << location << " / " << name << " (startSource = " << startSource
         << ", incrementSource = " << incrementSource << ", numberOfBuffers = " << numberOfBuffers << ")" << std::endl;
    }
  };

  /********************************************************************************************************************/

  struct XyDescription : public PropertyDescription, public PropertyAttributes {
    struct Axis {
      std::string label;
      int logarithmic;
      float start;
      float stop;
    };

    ChimeraTK::RegisterPath xSource;
    ChimeraTK::RegisterPath ySource;
    std::string description;
    std::map<std::string, Axis> axis;

    XyDescription(ChimeraTK::RegisterPath const& xSource_ = "", ChimeraTK::RegisterPath const& ySource_ = "",
        std::string const& location_ = "", std::string const& name_ = "", bool hasHistory_ = true)
    : PropertyDescription(location_, name_), PropertyAttributes(hasHistory_, false), xSource(xSource_),
      ySource(ySource_) {}

    const std::type_info& type() const override { return typeid(XyDescription); }

    void print(std::ostream& os = std::cout) const override {
      os << "x: " << xSource << " y: " << ySource << " -> " << location << " / " << name << std::endl;
    }
  };

  /********************************************************************************************************************/

  struct IfffDescription : public PropertyDescription, public PropertyAttributes {
    ChimeraTK::RegisterPath i1Source;
    ChimeraTK::RegisterPath f1Source, f2Source, f3Source;

    IfffDescription(ChimeraTK::RegisterPath const& i1Source_ = "", ChimeraTK::RegisterPath const& f1Source_ = "",
        ChimeraTK::RegisterPath const& f2Source_ = "", ChimeraTK::RegisterPath const& f3Source_ = "",
        std::string const& location_ = "", std::string const& name_ = "")
    : PropertyDescription(location_, name_), i1Source(i1Source_), f1Source(f1Source_), f2Source(f2Source_),
      f3Source(f3Source_) {}

    const std::type_info& type() const override { return typeid(IfffDescription); }

    void print(std::ostream& os = std::cout) const override {
      os << "i1: " << i1Source << ", f1: " << f1Source << ", f2: " << f2Source << ", f3: " << f3Source << " -> "
         << location << " / " << name << std::endl;
    }
  };

  /********************************************************************************************************************/

  // This is the per location information which are used as default for the
  // properties in this location
  struct LocationInfo : public PropertyAttributes {
    bool useHasHistoryDefault;
    bool useIsWriteableDefault;
    bool usePersistDefault = false;
    bool useMacroPulseNumberSourceDefault;
    bool useDataMatchingDefault;
    LocationInfo(bool useHasHistoryDefault_ = false, bool useIsWriteableDefault_ = false,
        bool useMacroPulseNumberSourceDefault_ = false, bool useDataMatchingDefault_ = false)
    : useHasHistoryDefault(useHasHistoryDefault_), useIsWriteableDefault(useIsWriteableDefault_),
      useMacroPulseNumberSourceDefault(useMacroPulseNumberSourceDefault_),
      useDataMatchingDefault(useDataMatchingDefault_) {}
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_PROPERTY_DESCRIPTION_H
