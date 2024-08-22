// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "PropertyDescription.h"
#include <unordered_set>

#include <ChimeraTK/RegisterPath.h>

#include <boost/any.hpp>

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace xmlpp {
  class Node;
  class Element;
  class Attribute;
} // namespace xmlpp

namespace ChimeraTK {

  class VariableMapper {
   public:
    static VariableMapper& getInstance();
    void prepareOutput(const std::string& xmlFile, std::set<std::string> inputVariables);

    [[nodiscard]] std::list<std::shared_ptr<PropertyDescription>> getPropertiesInLocation(
        const std::string& location) const;
    [[nodiscard]] std::list<std::shared_ptr<PropertyDescription>> const& getAllProperties() const;
    [[nodiscard]] std::unordered_set<std::string> getAllLocations() const;
    [[nodiscard]] std::map<std::string, int> getLocationAndCode() const { return _inputLocationAndCode; }

    VariableMapper(VariableMapper&) = delete;
    void operator=(VariableMapper const&) = delete;

    void directImport(std::set<std::string> inputVariables);

    // empty the created mapping
    void clear();

    /// printing the map is useful for debugging
    void print(std::ostream& os = std::cout) const;

    /// Function to get a bool out of the texts
    /// true/True/TRUE/false/False/FALSE/1/0. Note on the input: we intentionally
    /// make a copy because we modify it inside.
    static bool evaluateBool(std::string txt);

    /// Loop through the sub nodes and take the string from the first non-empty
    /// which can be casted to a string-node
    static std::string getContentString(xmlpp::Node const* node);

    /// Functiont o convert a string into a DataConsistencyGroup::MatchingMode enum value
    static DataConsistencyGroup::MatchingMode evaluateDataMatching(const std::string& txt);

    [[nodiscard]] const std::set<std::string>& getUsedVariables() const { return _usedInputVariables; }

    [[nodiscard]] const std::list<ErrorReportingInfo>& getErrorReportingInfos() const { return _errorReportingInfos; }

   protected:
    VariableMapper() = default;

    std::map<std::string, int> _inputLocationAndCode; // map of location and fct_code,
                                                      // from location tag of the mapping xml file
    std::set<std::string> _inputVariables;
    std::set<std::string> _usedInputVariables; // For tracing which variables are not to be imported.

    std::list<ErrorReportingInfo> _errorReportingInfos;

    void processLocationNode(xmlpp::Node const* locationNode);
    void processNode(xmlpp::Node const* propertyNode, const std::string& locationName);
    void processSpectrumNode(xmlpp::Node const* node, const std::string& locationName);
    void processImageNode(xmlpp::Node const* node, const std::string& locationName);
    void processXyNode(xmlpp::Node const* node, std::string& locationName);
    void processIfffNode(xmlpp::Node const* node, std::string& locationName);
    void processSetErrorNode(xmlpp::Node const* node, std::string& locationName);
    void processImportNode(xmlpp::Node const* importNode, const std::string& importLocationName = std::string());
    void processCode(xmlpp::Element const* location, const std::string& locationName);

    void import(std::string importSource, const std::string& importLocationName, const std::string& directory = "");
    bool getHasHistoryDefault(std::string const& locationName);
    bool getIsWriteableDefault(std::string const& locationName);
    PersistConfig getPersistDefault(std::string const& locationName);

    std::string getMacroPusleNumberSourceDefault(std::string const& locationName);
    DataConsistencyGroup::MatchingMode getDataMatchingDefault(std::string const& locationName);

    std::map<std::string, LocationInfo> _locationDefaults;
    PropertyAttributes _globalDefaults;

    // The created PropertyDescriptions
    std::list<std::shared_ptr<PropertyDescription>> _descriptions;

    /// An internal helper function to abbreviate the syntax
    static bool nodeIsWhitespace(const xmlpp::Node* node);

    // Check if the attribute exists (throw if not) and get it's content
    static std::string getAttributeValue(const xmlpp::Element* node, std::string const& attributeName);

    template<class PROPERTY_DESCRIPTION_TYPE>
    void processHistoryAndWritableAttributes(PROPERTY_DESCRIPTION_TYPE propertyDescription,
        const xmlpp::Element* propertyXmlElement, const std::string& locationName);

    void addDescription(
        std::shared_ptr<PropertyDescription> const& propertyDescription, std::list<std::string> const& absoluteSources);
  };

} // namespace ChimeraTK
