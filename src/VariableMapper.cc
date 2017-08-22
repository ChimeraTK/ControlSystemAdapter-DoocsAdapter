#include "VariableMapper.h"

#include <libxml++/libxml++.h>
#include <regex>
#include "splitStringAtFirstSlash.h"
#include <mtca4u/RegisterPath.h>
#include <iostream>
#include <locale>
#include <algorithm>

namespace ChimeraTK{
  
  VariableMapper & VariableMapper::getInstance(){
    static VariableMapper instance;
    return instance;
  }

  bool VariableMapper::nodeIsWhitespace(const xmlpp::Node* node){
    const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
    if(nodeAsText){
      return nodeAsText->is_white_space();
    }
    return false;
  }

  void VariableMapper::processLocationNode(xmlpp::Node const * locationNode){
    const xmlpp::Element* location = dynamic_cast<const xmlpp::Element*>(locationNode);
    std::string locationName = location->get_attribute("name")->get_value();

    for (auto const & node : location->get_children()){
        if (nodeIsWhitespace(node)) continue;
        if (dynamic_cast<xmlpp::CommentNode const *>(node)) continue;        

        if (node->get_name() == "property"){
          processPropertyNode(node, locationName);
        }else if (node->get_name() == "import"){
          processImportNode(node, locationName);
        }else if (node->get_name() == "has_history"){
          auto & locationInfo = _locationDefaults[locationName];
          locationInfo.useHasHistoryDefault = true;
          locationInfo.hasHistory = evaluateBool(getContentString(node));
        }else if (node->get_name() == "is_writeable"){
          auto & locationInfo = _locationDefaults[locationName];
          locationInfo.useIsWriteableDefault = true;
          locationInfo.isWriteable = evaluateBool(getContentString(node));
        }else{
          throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName + ": Unknown node '"+node->get_name()+"'");
        }
    }
  }

  void VariableMapper::processPropertyNode(xmlpp::Node const * propertyNode, std::string locationName){
    const xmlpp::Element* property = dynamic_cast<const xmlpp::Element*>(propertyNode);

    std::string source = property->get_attribute("source")->get_value();

    std::string name;
    const xmlpp::Attribute* nameAttribute = property->get_attribute("name");
    if (nameAttribute){
      name = nameAttribute->get_value();
    }else{
      if (source[0] == '/'){
        name = source.substr(1);
      }else{
        name = source;
      }
      // replace / with . in name
      name = std::regex_replace(name, std::regex("/"), ".");
    }

    // prepare the property description
    PropertyDescription propertyDescription(locationName, name);

    auto hasHistoryNodes = property->get_children("has_history");
    if (!hasHistoryNodes.empty()){
      propertyDescription.hasHistory= evaluateBool(getContentString(hasHistoryNodes.front()));
    }else{
      propertyDescription.hasHistory = getHasHistoryDefault(locationName);
    }
    auto isWriteableNodes = property->get_children("is_writeable");
    if (!isWriteableNodes.empty()){
      propertyDescription.isWriteable = evaluateBool(getContentString(isWriteableNodes.front()));
    }else{
      propertyDescription.isWriteable = getIsWriteableDefault(locationName);
    }

    std::string absoluteSource;
    if (source[0] == '/'){
      absoluteSource=source;
    }else{
      absoluteSource=std::string("/")+locationName+"/"+source;
    }

    auto existingCandidate = _inputSortedDescriptions.find(absoluteSource);
    if (existingCandidate != _inputSortedDescriptions.end()){
      auto existingPropertyDescription = existingCandidate->second;
      throw std::logic_error(std::string("Invalid XML content for ") + absoluteSource + " -> "+ locationName+"/" +name +". Process variable already defined to point to " + existingPropertyDescription.location + "/" + existingPropertyDescription.name);
    }else{
      _inputSortedDescriptions[absoluteSource] = propertyDescription;
    }
   
  }

  void VariableMapper::processImportNode(xmlpp::Node const * importNode, std::string importLocationName){
    const xmlpp::Element* importElement = dynamic_cast<const xmlpp::Element*>(importNode);
    std::string directory;
    if (importElement){
      // look for a directory attribute
      const xmlpp::Attribute* directoryAttribute = importElement->get_attribute("directory");
      if (directoryAttribute){
        directory = directoryAttribute->get_value();
      }
    }
    
    for (auto const & node : importNode->get_children()){
      const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
      std::string importSource = nodeAsText->get_content();
      import(importSource, importLocationName, directory);
    }
   }

  void VariableMapper::import(std::string importSource, std::string importLocationName,
                              std::string directory){
    // a slash will be added after the source, so we make the source empty for an import of everything
    if (importSource == "/"){
      importSource = "";
    }
     
    // loop source tree, cut beginning, replace / with _ and add a property
    for (auto const & processVariable : _inputVariables){
      if (_inputSortedDescriptions.find(processVariable) != _inputSortedDescriptions.end()){
        continue;
      }
        
      if ( processVariable.find( importSource+"/") == 0 ){
        // processVariable starts with wanted source
        auto nameSource = processVariable.substr( importSource.size() + 1); // add the slash to be removed
        // we use the register path because it removes duplicate separators and allows to use
        // . as separater to replace all / with .
        mtca4u::RegisterPath propertyName;
        std::string locationName;
        if (importLocationName.empty()){
          // a global import, try to get the location name from the source
          auto locationAndPropertyName = splitStringAtFirstSlash(nameSource);
          locationName = locationAndPropertyName.first;
          propertyName = locationAndPropertyName.second;
          if (locationName.empty() ){
            throw std::logic_error(std::string("Invalid XML content in global import of ") +  (importSource.empty()?"/":importSource) + ":  Cannot create location name from '" + nameSource + "', one hirarchy level is missing.");
          }
          // convenience for the user: You get an error message is you try a global import
          // with directory (in case you did not validate your xml against the schema).
          if (!directory.empty()){
            throw std::logic_error(std::string("Invalid XML content in global import of ") + (importSource.empty()?"/":importSource) + ":  You cannot have a directory in a global import.");
          }
        }else{
          // import into a location, we know the location name.
          // add the directory first, then add the name source property
          propertyName /= directory;
          propertyName /= nameSource;
          locationName = importLocationName;
        }

        // get the property name with . instead of /
        propertyName.setAltSeparator(".");
        PropertyDescription propertyDescription(locationName, propertyName.getWithAltSeparator());

        // we are importing, so all properties get the intended defaults (not individual settings)
        propertyDescription.hasHistory = getHasHistoryDefault(locationName);
        propertyDescription.isWriteable = getIsWriteableDefault(locationName);
        
        _inputSortedDescriptions[processVariable] = propertyDescription;
      }
    }
  }

  void VariableMapper::prepareOutput(std::string xmlFile, std::set< std::string > inputVariables){
    clear();
    _inputVariables=inputVariables;
    
    xmlpp::DomParser parser;
    //    parser.set_validate();
    parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
    parser.parse_file(xmlFile);
    
    if(parser){
      //Walk the tree:
      const xmlpp::Node* rootNode = parser.get_document()->get_root_node(); //deleted by DomParser.
      
      for (auto const & mainNode : rootNode->get_children()){
        if (nodeIsWhitespace(mainNode)) continue;
        
        if (mainNode->get_name() == "location"){
          processLocationNode(mainNode);
        }else if (mainNode->get_name() == "import"){
          processImportNode(mainNode);
        }else if (mainNode->get_name() == "has_history"){
          _globalDefaults.hasHistory = evaluateBool(getContentString(mainNode));
        }else if (mainNode->get_name() == "is_writeable"){
          _globalDefaults.isWriteable = evaluateBool(getContentString(mainNode));
        }else{
          throw std::invalid_argument(std::string("Error parsing xml file ") + xmlFile + ": Unknown node '"+mainNode->get_name()+"'");
        }
      }

    }else{
      throw std::invalid_argument(std::string("Error parsing xml file ") + xmlFile + ". No document produced.");
    }
  }

  std::map< std::string, VariableMapper::PropertyDescription > const & VariableMapper::getAllProperties() const{
    return _inputSortedDescriptions;
  }

  std::map< std::string, VariableMapper::PropertyDescription > VariableMapper::getPropertiesInLocation(std::string location) const{
    std::map< std::string, PropertyDescription > output;

    for (auto const & variable : _inputSortedDescriptions){
      if (variable.second.location == location){
        // no need to check return value. There cannot be duplicate entries because the values are
        // coming from another map.
        (void) output.insert( variable );
      }
    }
    return output;
  }

  void VariableMapper::directImport(std::set< std::string > inputVariables){
    clear();
    _inputVariables=inputVariables;
    import("/",""); // import from /, create location names from first level of the tree
  }

  void VariableMapper::clear(){
    _inputVariables.clear();
    _locationDefaults.clear();
    _globalDefaults = PropertyAttributes();
    _inputSortedDescriptions.clear();
  }

  /// printing the map is useful for debugging
  void VariableMapper::print(std::ostream & os) const {
    for (auto & variableNameAndPropertyDescription : _inputSortedDescriptions ){
      auto & variableName = variableNameAndPropertyDescription.first;
      auto & propertyDescription = variableNameAndPropertyDescription.second;
      os << variableName << " -> " << propertyDescription.location << " / " << propertyDescription.name
         << " hasHistory:" << propertyDescription.hasHistory
         << " isWriteable:" << propertyDescription.isWriteable
         << std::endl;
    }
  }

  bool VariableMapper::evaluateBool(std::string txt){
    transform(txt.begin(), txt.end(), txt.begin(), ::tolower);
    if (txt=="false" or txt=="0"){
      return false;
    }else if (txt=="true" or txt=="1"){
      return true;
    }else{
      throw std::invalid_argument(std::string("Error parsing xml file: could not convert to bool: ") + txt);
    }
  }

  std::string VariableMapper::getContentString(xmlpp::Node const *node){
    for (auto const & subNode : node->get_children()){
      const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(subNode);
      if(nodeAsText){
        if (nodeAsText->is_white_space()){
          continue;
        }
        return nodeAsText->get_content();
      }
    }
    throw std::invalid_argument(std::string("Error parsing xml file: node ") + node->get_name() +
                                " does not have text nodes as children."); 
  }

  bool VariableMapper::getHasHistoryDefault(std::string const & locationName){
    // if there is no default setting for the location, we will get the "default default" which
    // has useHasHistoryDefault disabled which is auto-generated by the [] operator
    auto locationInfo  = _locationDefaults[locationName];
    if (locationInfo.useHasHistoryDefault){
      return locationInfo.hasHistory;
    }else{
      return _globalDefaults.hasHistory;
    }
  }

  bool VariableMapper::getIsWriteableDefault(std::string const & locationName){
    // if there is no default setting for the location, we will get the "default default" which
    // has useHasHistoryDefault disabled which is auto-generated by the [] operator
    auto locationInfo  = _locationDefaults[locationName];
    if (locationInfo.useIsWriteableDefault){
      return locationInfo.isWriteable;
    }else{
      return _globalDefaults.isWriteable;
    }
  }

} // namespace ChimeraTK

