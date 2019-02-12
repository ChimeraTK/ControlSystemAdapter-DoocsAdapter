#ifndef CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
#define CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H

#include <string>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <iostream>
#include <boost/any.hpp>
#include <ChimeraTK/RegisterPath.h>

#include "PropertyDescription.h"

namespace xmlpp {
  class Node;
  class Element;
} // namespace xmlpp

namespace ChimeraTK {

  class VariableMapper {
   public:
    static VariableMapper& getInstance();
    void prepareOutput(std::string xmlFile, std::set<std::string> inputVariables);

    std::list<std::shared_ptr<PropertyDescription>> getPropertiesInLocation(std::string location) const;
    std::list<std::shared_ptr<PropertyDescription>> const& getAllProperties() const;

    VariableMapper(VariableMapper&) = delete;
    void operator=(VariableMapper const&) = delete;

    void directImport(std::set<std::string> inputVariables);

    // empty the created mapping
    void clear();

    /// printing the map is useful for debugging
    void print(std::ostream& os = std::cout) const;

    /// Function to get a bool out of the texts true/True/TRUE/false/False/FALSE/1/0.
    /// Note on the input: we intentionally make a copy because we modify it inside.
    static bool evaluateBool(std::string txt);

    /// Loop through the sub nodes and take the string from the first non-empty
    /// which can be casted to a string-node
    static std::string getContentString(xmlpp::Node const* node);

   protected:
    VariableMapper() = default;

    std::set<std::string> _inputVariables;
    std::set<std::string> _usedInputVariables; // For tracing which variables are not to be imported.

    void processLocationNode(xmlpp::Node const* locationNode);
    void processPropertyNode(xmlpp::Node const* propertyNode, std::string locationName);
    void processSpectrumNode(xmlpp::Node const* node, std::string locationName);
    void processArrayNode(xmlpp::Node const* node, std::string locationName);
    void processImportNode(xmlpp::Node const* importNode, std::string importLocationName = std::string());

    void import(std::string importSource, std::string importLocationName, std::string directory = "");
    bool getHasHistoryDefault(std::string const& locationName);
    bool getIsWriteableDefault(std::string const& locationName);
    std::string getMacroPusleNumberSourceDefault(std::string const& locationName);

    std::map<std::string, LocationInfo> _locationDefaults;
    PropertyAttributes _globalDefaults;

    // The created PropertyDescriptions
    std::list<std::shared_ptr<PropertyDescription>> _descriptions;

    /// An internal helper function to abbreviate the syntax
    bool nodeIsWhitespace(const xmlpp::Node* node);

    // Check if the attribute exists (throw if not) and get it's content
    std::string getAttributeValue(const xmlpp::Element* node, std::string const& attributeName);

    template<class PROPERTY_DESCRIPTION_TYPE>
    void processHistoryAndWritableAttributes(PROPERTY_DESCRIPTION_TYPE propertyDescription,
        const xmlpp::Element* propertyXmlElement,
        std::string locationName);

    void addDescription(std::shared_ptr<PropertyDescription> const& propertyDescription,
        std::list<std::string> const& absoluteSoures);
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
