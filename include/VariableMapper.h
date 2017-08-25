#ifndef CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
#define CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H

#include <string>
#include <map>
#include <set>
#include <memory>
#include <iostream>
#include <boost/any.hpp>
#include <mtca4u/RegisterPath.h>

#include "PropertyDescription.h"

namespace xmlpp{
  class Node;
}

namespace ChimeraTK{

  class VariableMapper{
  public:
    static VariableMapper & getInstance();
    void prepareOutput(std::string xmlFile, std::set< std::string > inputVariables);

    std::map< std::string, std::shared_ptr<PropertyDescription> > getPropertiesInLocation(std::string location) const;
    std::map< std::string, std::shared_ptr<PropertyDescription> > const & getAllProperties() const;

    VariableMapper(VariableMapper &)=delete;
    void operator=(VariableMapper const &)=delete;

    void directImport(std::set< std::string > inputVariables);
    
    // empty the created mapping 
    void clear();

    /// printing the map is useful for debugging
    void print(std::ostream & os = std::cout) const;
    
    /// Function to get a bool out of the texts true/True/TRUE/false/False/FALSE/1/0.
    /// Note on the input: we intentionally make a copy because we modify it inside.
    static bool evaluateBool(std::string txt);

    /// Loop through the sub nodes and take the string from the first non-empty
    /// which can be casted to a string-node
    static std::string getContentString(xmlpp::Node const *node);
     
  protected:
    VariableMapper()=default;

    std::set< std::string > _inputVariables;
  
    void processLocationNode(xmlpp::Node const * locationNode);
    void processPropertyNode(xmlpp::Node const * propertyNode, std::string locationName);
    void processImportNode(xmlpp::Node const * importNode, std::string importLocationName=std::string());

    void import(std::string importSource, std::string importLocationName, std::string directory="");
    bool getHasHistoryDefault(std::string const & locationName);
    bool getIsWriteableDefault(std::string const & locationName);
   
    std::map<std::string, LocationInfo> _locationDefaults;
    PropertyAttributes _globalDefaults;

    // PropertyDescriptions, sorted by input, i.e. the ChimeraTK PV name
    std::map<std::string, std::shared_ptr<PropertyDescription> > _inputSortedDescriptions;

    /// An internal helper function to abbreviate the syntax
    bool nodeIsWhitespace(const xmlpp::Node* node);
  };

} // namespace ChimeraTK

#endif // CHIMERATK_DOOCS_ADAPTER_VARIABLE_MAPPER_H
