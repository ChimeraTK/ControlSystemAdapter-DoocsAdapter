#include "VariableMapper.h"

#include <libxml++/libxml++.h>
#include <regex>
#include "splitStringAtFirstSlash.h"

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

  void VariableMapper::processLocation(xmlpp::Node const * locationNode){
    const xmlpp::Element* location = dynamic_cast<const xmlpp::Element*>(locationNode);
    std::string locationName = location->get_attribute("name")->get_value();

    for (auto const & node : location->get_children()){
        if (nodeIsWhitespace(node)) continue;
        
        if (node->get_name() == "property"){
          processProperty(node, locationName);
        }else if (node->get_name() == "import"){
          processImport(node, locationName);
        }else{
          throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName + ": Unknown node '"+node->get_name()+"'");
        }
    }
  }

  void VariableMapper::processProperty(xmlpp::Node const * propertyNode, std::string locationName){
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
      _inputSortedDescriptions[absoluteSource] = PropertyDescription(locationName, name);
    }
   
  }

  void VariableMapper::processImport(xmlpp::Node const * importNode, std::string importLocationName){
    for (auto const & node : importNode->get_children()){
      const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
      std::string importSource = nodeAsText->get_content();

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
          std::string propertyName;
          std::string locationName;
          if (importLocationName.empty()){
            // a global import, try to get the location name from the source
            auto locationAndPropertyName = splitStringAtFirstSlash(nameSource);
            locationName = locationAndPropertyName.first;
            propertyName = locationAndPropertyName.second;
            if (locationName.empty() ){
              throw std::logic_error(std::string("Invalid XML content in global import of ") + importSource + ":  Cannot create location name from '" + nameSource + "', one hirarchy level is missing.");
            }
          }else{
            // import into a location, we know the location name. Just replace / with .
            propertyName = std::regex_replace(nameSource, std::regex("/"), ".");
            locationName = importLocationName;
          }

          _inputSortedDescriptions[processVariable] = PropertyDescription(locationName, propertyName);
        }
      }
    }
  }

  void VariableMapper::prepareOutput(std::string xmlFile, std::set< std::string > inputVariables){
    _inputVariables=inputVariables;
    
    xmlpp::DomParser parser;
    //    parser.set_validate();
    parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
    parser.parse_file(xmlFile);
    
    if(parser){
      _locationDefaults.clear();
      _globalDefaults = PropertyAttributes();
      _inputSortedDescriptions.clear();
        
      //Walk the tree:
      const xmlpp::Node* rootNode = parser.get_document()->get_root_node(); //deleted by DomParser.
      
      for (auto const & mainNode : rootNode->get_children()){
        if (nodeIsWhitespace(mainNode)) continue;
        
        if (mainNode->get_name() == "location"){
          processLocation(mainNode);
        }else if (mainNode->get_name() == "import"){
          processImport(mainNode);
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


} // namespace ChimeraTK

