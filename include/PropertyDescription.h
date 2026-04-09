// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Utilities.h"

#include <ChimeraTK/DataConsistencyGroup.h>
#include <ChimeraTK/RegisterPath.h>

#include <string>
#include <utility>

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

    // NOLINTNEXTLINE(google-explicit-constructor)
    PersistConfig(std::string txt) {
      transform(txt.begin(), txt.end(), txt.begin(), ::tolower);
      if(txt == "false" or txt == "off" or txt == "0") {
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
    // NOLINTNEXTLINE(google-explicit-constructor)
    PersistConfig(int enumVal) { this->val = enumVal; }
    bool operator==(const PersistConfig& other) const { return val == other.val; }
  };

  // PropertyAttributes are used in the property description itself, and
  // as default values (global and in the locations)
  struct PropertyAttributes {
    bool hasHistory;
    bool isWriteable;
    bool publishZMQ;
    std::string macroPulseNumberSource;
    std::string isWriteableSource;
    DataConsistencyGroup::MatchingMode dataMatching;
    PersistConfig persist = PersistConfig::ON;
    explicit PropertyAttributes(bool hasHistory_ = true, bool isWriteable_ = true, bool publishZMQ_ = false,
        std::string macroPulseNumberSource_ = "", std::string isWriteableSource_ = "",
        DataConsistencyGroup::MatchingMode dataMatching_ = DataConsistencyGroup::MatchingMode::exact)
    : hasHistory(hasHistory_), isWriteable(isWriteable_), publishZMQ(publishZMQ_),
      macroPulseNumberSource(std::move(macroPulseNumberSource_)), isWriteableSource(std::move(isWriteableSource_)),
      dataMatching(dataMatching_) {}
    bool operator==(PropertyAttributes const& other) const {
      return (hasHistory == other.hasHistory && isWriteable == other.isWriteable && publishZMQ == other.publishZMQ &&
          macroPulseNumberSource == other.macroPulseNumberSource && dataMatching == other.dataMatching &&
          persist == other.persist);
    }
  };

  /********************************************************************************************************************/

  // Common for all properties, the base class to be stored.
  // FIXME: should sort by name to put it into a set?
  struct PropertyDescription : public PropertyAttributes {
    std::string location;
    std::string name;
    explicit PropertyDescription(
        std::string location_ = "", std::string name_ = "", const PropertyAttributes& propAttr = PropertyAttributes{})
    : PropertyAttributes(propAttr), location(std::move(location_)), name(std::move(name_)) {}
    virtual ~PropertyDescription() = default;
    virtual bool operator==(PropertyDescription const& other) const {
      return location == other.location && name == other.name;
    }
    virtual void print(std::ostream& os = std::cout) const { os << location << " / " << name << std::endl; }

    // Return all PV names which are used (read or write) by the property.
    std::set<std::string> getSources() {
      auto sources = getMetadataSources();
      auto payload = getPayloadDataSources();
      sources.insert(payload.begin(), payload.end());
      return sources;
    }

    std::set<std::string> getReadSources() {
      auto sources = getMetadataSources();
      if(!isWriteable) {
        auto payload = getPayloadDataSources();
        sources.insert(payload.begin(), payload.end());
      }
      return sources;
    }

    std::set<std::string> getWriteSources() {
      if(isWriteable) {
        return getPayloadDataSources();
      }
      return {};
    }

    virtual std::set<std::string> getMetadataSources() {
      std::set<std::string> sources;
      if(!macroPulseNumberSource.empty()) {
        sources.insert(getAbsoluteSource(macroPulseNumberSource, location));
      }
      if(!isWriteableSource.empty()) {
        sources.insert(getAbsoluteSource(isWriteableSource, location));
      }
      return sources;
    }

    virtual std::set<std::string> getPayloadDataSources() = 0;
  };

  /********************************************************************************************************************/

  // Combines property attributes and the base description
  // FIXME: should sort by name to put it into a set?
  struct AutoPropertyDescription : public PropertyDescription {
    enum class DataType { Byte, Short, Int, Long, Float, Double, Bool, Void, Auto };
    ChimeraTK::RegisterPath source;
    explicit AutoPropertyDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "",
        std::string name_ = "", DataType dataType_ = DataType::Auto, bool hasHistory_ = true, bool isWriteable_ = true)
    : PropertyDescription(std::move(location_), std::move(name_), PropertyAttributes(hasHistory_, isWriteable_)),
      source(source_), dataType(dataType_) {}
    bool operator==(PropertyDescription const& other) const override {
      if(typeid(other) == typeid(AutoPropertyDescription)) {
        auto casted_other = dynamic_cast<AutoPropertyDescription const&>(other);
        return dataType == casted_other.dataType && source == casted_other.source && location == other.location &&
            name == other.name && static_cast<const PropertyAttributes*>(this)->operator==(casted_other);
      }
      return false;
    }
    void print(std::ostream& os = std::cout) const override {
      os << source << " -> " << location << " / " << name << std::endl;
    }

    void deriveType(std::type_info const& info) {
      if(info == typeid(uint8_t) || info == typeid(int8_t)) {
        dataType = DataType::Byte;
      }
      if(info == typeid(uint16_t) || info == typeid(int16_t)) {
        dataType = DataType::Short;
      }
      if(info == typeid(uint32_t) || info == typeid(int32_t)) {
        dataType = DataType::Int;
      }
      if(info == typeid(uint64_t) || info == typeid(int64_t)) {
        dataType = DataType::Long;
      }
      if(info == typeid(float)) {
        dataType = DataType::Float;
      }
      if(info == typeid(double)) {
        dataType = DataType::Double;
      }
      if(info == typeid(ChimeraTK::Boolean)) {
        dataType = DataType::Bool;
      }
      if(info == typeid(ChimeraTK::Void)) {
        dataType = DataType::Void;
      }
    }

    std::set<std::string> getPayloadDataSources() override { return {source}; };

    DataType dataType;
  };

  /********************************************************************************************************************/

  struct ImageDescription : public PropertyDescription {
    ChimeraTK::RegisterPath source;
    std::string description;

    explicit ImageDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "",
        std::string name_ = "", bool hasHistory_ = false, bool isWriteable_ = false)
    : PropertyDescription(std::move(location_), std::move(name_), PropertyAttributes(hasHistory_, isWriteable_)),
      source(source_) {}

    void print(std::ostream& os = std::cout) const override {
      os << source << " -> " << location << " / " << name << std::endl;
    }

    std::set<std::string> getPayloadDataSources() override { return {source}; };
  };

  /********************************************************************************************************************/

  struct SpectrumDescription : public PropertyDescription {
    struct Axis {
      std::string label;
      int logarithmic{};
      float start{};
      float stop{};
    };

    ChimeraTK::RegisterPath source;
    ChimeraTK::RegisterPath startSource;
    ChimeraTK::RegisterPath incrementSource;
    float start{0};
    float increment{1.0};
    size_t numberOfBuffers{1};
    std::string description;
    std::map<std::string, Axis> axis;

    explicit SpectrumDescription(ChimeraTK::RegisterPath const& source_ = "", std::string location_ = "",
        std::string name_ = "", bool hasHistory_ = true, bool isWriteable_ = true)
    : PropertyDescription(std::move(location_), std::move(name_), PropertyAttributes(hasHistory_, isWriteable_)),
      source(source_) {}

    void print(std::ostream& os = std::cout) const override {
      os << source << " -> " << location << " / " << name << " (startSource = " << startSource
         << ", incrementSource = " << incrementSource << ", numberOfBuffers = " << numberOfBuffers << ")" << std::endl;
    }

    std::set<std::string> getPayloadDataSources() override { return {source}; };

    std::set<std::string> getMetadataSources() override {
      std::set<std::string> baseSet = PropertyDescription::getMetadataSources();
      if(startSource.length() > 1) {
        baseSet.insert(startSource);
      }
      if(incrementSource.length() > 1) {
        baseSet.insert(incrementSource);
      }
      return baseSet;
    };
  };

  /********************************************************************************************************************/

  struct XyDescription : public PropertyDescription {
    struct Axis {
      std::string label;
      int logarithmic{};
      float start{};
      float stop{};
    };

    ChimeraTK::RegisterPath xSource;
    ChimeraTK::RegisterPath ySource;
    std::string description;
    std::map<std::string, Axis> axis;

    explicit XyDescription(ChimeraTK::RegisterPath const& xSource_ = "", ChimeraTK::RegisterPath const& ySource_ = "",
        std::string const& location_ = "", std::string const& name_ = "", bool hasHistory_ = true)
    : PropertyDescription(location_, name_, PropertyAttributes(hasHistory_, false)), xSource(xSource_),
      ySource(ySource_) {}

    void print(std::ostream& os = std::cout) const override {
      os << "x: " << xSource << " y: " << ySource << " -> " << location << " / " << name << std::endl;
    }

    std::set<std::string> getPayloadDataSources() override { return {xSource, ySource}; };
  };

  /********************************************************************************************************************/

  struct IfffDescription : public PropertyDescription {
    ChimeraTK::RegisterPath i1Source;
    ChimeraTK::RegisterPath f1Source, f2Source, f3Source;

    explicit IfffDescription(ChimeraTK::RegisterPath const& i1Source_ = "",
        ChimeraTK::RegisterPath const& f1Source_ = "", ChimeraTK::RegisterPath const& f2Source_ = "",
        ChimeraTK::RegisterPath const& f3Source_ = "", std::string const& location_ = "", std::string const& name_ = "")
    : PropertyDescription(location_, name_), i1Source(i1Source_), f1Source(f1Source_), f2Source(f2Source_),
      f3Source(f3Source_) {}

    void print(std::ostream& os = std::cout) const override {
      os << "i1: " << i1Source << ", f1: " << f1Source << ", f2: " << f2Source << ", f3: " << f3Source << " -> "
         << location << " / " << name << std::endl;
    }

    std::set<std::string> getPayloadDataSources() override { return {i1Source, f1Source, f2Source, f3Source}; };
  };

  /********************************************************************************************************************/

  struct IiiiDescription : public PropertyDescription {
    ChimeraTK::RegisterPath iiiiSource;

    IiiiDescription(ChimeraTK::RegisterPath const& iiiiSource_ = "", std::string const& location_ = "",
        std::string const& name_ = "")
    : PropertyDescription(location_, name_), iiiiSource(iiiiSource_) {};

    void print(std::ostream& os = std::cout) const override {
      os << "iiii: " << iiiiSource << " -> " << location << " / " << name << std::endl;
    }

    std::set<std::string> getPayloadDataSources() override { return {iiiiSource}; };
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
    explicit LocationInfo(bool useHasHistoryDefault_ = false, bool useIsWriteableDefault_ = false,
        bool useMacroPulseNumberSourceDefault_ = false, bool useDataMatchingDefault_ = false)
    : useHasHistoryDefault(useHasHistoryDefault_), useIsWriteableDefault(useIsWriteableDefault_),
      useMacroPulseNumberSourceDefault(useMacroPulseNumberSourceDefault_),
      useDataMatchingDefault(useDataMatchingDefault_) {}
  };

  // parsed info about error reporting to locations
  struct ErrorReportingInfo {
    ChimeraTK::RegisterPath statusCodeSource;
    ChimeraTK::RegisterPath statusStringSource;
    std::string targetLocation;
  };

} // namespace ChimeraTK
