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
    if (!location){
      throw std::invalid_argument("Error parsing xml file in location node.");
    }
    std::string locationName = getAttributeValue(location,"name");

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
        }else if (node->get_name() == "D_spectrum"){
          processSpectrumNode(node, locationName);
        }else if (node->get_name() == "D_array"){
          processArrayNode(node, locationName);
        }else{
          throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName + ": Unknown node '"+node->get_name()+"'");
        }
    }
  }

  std::string determineName(const xmlpp::Element* property, std::string source){
    const xmlpp::Attribute* nameAttribute = property->get_attribute("name");
    // if a name is given in xml take it
    if (nameAttribute){
      return nameAttribute->get_value();
    }else{// auto-determine the name from the source
      ///@todo FIXME: use register path to do the slash and replace fiddeling
      std::string name;
      if (source[0] == '/'){
        name = source.substr(1);
      }else{
        name = source;
      }
      // replace / with . in name
      return std::regex_replace(name, std::regex("/"), ".");
    }
  }

  std::string getAbsoluteSource(std::string source, std::string locationName){
    if (source[0] == '/'){
      return source;
    }else{
      return std::string("/")+locationName+"/"+source;
    }
  }

  template<class PROPERTY_DESCRIPTION_TYPE>
  void VariableMapper::processHistoryAndWritableAttributes(PROPERTY_DESCRIPTION_TYPE propertyDescription,  const xmlpp::Element* propertyXmlElement, std::string locationName){
    auto hasHistoryNodes = propertyXmlElement->get_children("has_history");
    if (!hasHistoryNodes.empty()){
      propertyDescription->hasHistory= evaluateBool(getContentString(hasHistoryNodes.front()));
    }else{
      propertyDescription->hasHistory = getHasHistoryDefault(locationName);
    }
    auto isWriteableNodes = propertyXmlElement->get_children("is_writeable");
    if (!isWriteableNodes.empty()){
      propertyDescription->isWriteable = evaluateBool(getContentString(isWriteableNodes.front()));
    }else{
      propertyDescription->isWriteable = getIsWriteableDefault(locationName);
    }

  }

  void VariableMapper::addDescription(std::shared_ptr<PropertyDescription> const & propertyDescription, std::list< std::string > const & absoluteSources){
    _descriptions.push_back( propertyDescription );
    for (auto & source : absoluteSources ){
      _usedInputVariables.insert(source);
    }
  }

  xmlpp::Element const * asXmlElement(xmlpp::Node const * node){
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
    if (!element){
      throw std::invalid_argument("Error parsing xml file: Node is not an element node: " + node->get_name());
    }
    return element;
  }
  
  void VariableMapper::processPropertyNode(xmlpp::Node const * propertyNode, std::string locationName){
    auto property = asXmlElement(propertyNode);

    std::string source = getAttributeValue(property, "source");
    std::string name = determineName(property, source);
    std::string absoluteSource = getAbsoluteSource(source, locationName);

    // prepare the property description
    auto autoPropertyDescription = std::make_shared<AutoPropertyDescription>(absoluteSource, locationName, name);

    processHistoryAndWritableAttributes(autoPropertyDescription, property, locationName);

    addDescription(autoPropertyDescription, {absoluteSource});
  }


  void VariableMapper::processSpectrumNode(xmlpp::Node const * node, std::string locationName){
    auto spectrumXml = asXmlElement(node);

    // the "main source" of a spectum
    std::string source = getAttributeValue(spectrumXml, "source");
    std::string name = determineName(spectrumXml, source);
    std::string absoluteSource = getAbsoluteSource(source, locationName);

    // prepare the property description
    auto spectrumDescription = std::make_shared<SpectrumDescription>(absoluteSource, locationName, name);

    processHistoryAndWritableAttributes(spectrumDescription, spectrumXml, locationName);

    auto startNodes = spectrumXml->get_children("start");
    if (!startNodes.empty()){
      spectrumDescription->start = std::stof(getContentString(startNodes.front()));
    }
    std::list< std::string > usedVariables({absoluteSource});
    auto incrementNodes = spectrumXml->get_children("increment");
    if (!incrementNodes.empty()){
      spectrumDescription->increment = std::stof(getContentString(incrementNodes.front()));
    }
    auto startSourceNodes = spectrumXml->get_children("startSource");
    if (!startSourceNodes.empty()){
      auto startSource = getContentString(startSourceNodes.front());
      spectrumDescription->startSource = startSource;
      usedVariables.push_back(startSource);
     }
    auto incrementSourceNodes = spectrumXml->get_children("incrementSource");
    if (!incrementSourceNodes.empty()){
      auto incrementSource = getContentString(incrementSourceNodes.front());
      spectrumDescription->incrementSource = incrementSource;
      usedVariables.push_back(incrementSource);
    }

    addDescription(spectrumDescription, usedVariables);
 }

  void VariableMapper::processArrayNode(xmlpp::Node const * node, std::string locationName){
    auto arrayXml = asXmlElement(node);

    // the "main source" of a spectum
    std::string source = getAttributeValue(arrayXml, "source");
    std::string name = determineName(arrayXml, source);

    const xmlpp::Attribute* typeAttribute = arrayXml->get_attribute("type");
    std::map< std::string, ArrayDescription::DataType > dataTypeMap(
      { {"auto", ArrayDescription::DataType::Auto},
        {"byte", ArrayDescription::DataType::Byte},
        {"short", ArrayDescription::DataType::Short},
        {"int", ArrayDescription::DataType::Int},
        {"long", ArrayDescription::DataType::Long},
        {"float", ArrayDescription::DataType::Float},
        {"double", ArrayDescription::DataType::Double} });
    
    ArrayDescription::DataType type;
    if (typeAttribute){
      type = dataTypeMap[typeAttribute->get_value()];
    }else{
      type = ArrayDescription::DataType::Auto;
    }
    
    std::string absoluteSource = getAbsoluteSource(source, locationName);

    // prepare the property description
    auto arrayDescription = std::make_shared<ArrayDescription>(absoluteSource, locationName, name, type);

    processHistoryAndWritableAttributes(arrayDescription, arrayXml, locationName);

    addDescription(arrayDescription, {absoluteSource});
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
      if (_usedInputVariables.find(processVariable) != _usedInputVariables.end()){
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
        auto autoPropertyDescription = std::make_shared<AutoPropertyDescription>(processVariable, locationName, propertyName.getWithAltSeparator());

        // we are importing, so all properties get the intended defaults (not individual settings)
        autoPropertyDescription->hasHistory = getHasHistoryDefault(locationName);
        autoPropertyDescription->isWriteable = getIsWriteableDefault(locationName);
        
        addDescription(autoPropertyDescription, {processVariable});
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
        if (dynamic_cast<xmlpp::CommentNode const *>(mainNode)) continue;        
        
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

  std::list< std::shared_ptr<PropertyDescription> > const & VariableMapper::getAllProperties() const{
    return _descriptions;
  }

  std::list< std::shared_ptr<PropertyDescription> > VariableMapper::getPropertiesInLocation(std::string location) const{
    std::list< std::shared_ptr<PropertyDescription> > output;

    for (auto const & variable : _descriptions){
      if (variable->location == location){
        (void) output.push_back( variable );
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
    _usedInputVariables.clear();
    _locationDefaults.clear();
    _globalDefaults = PropertyAttributes();
    _descriptions.clear();
  }

  /// printing the map is useful for debugging
  void VariableMapper::print(std::ostream & os) const {
    os << "====== VariableMapper =====" << std::endl;
    for (auto & description : _descriptions ){
      description->print(os);
    }
    os << "======= Mapping End =======" << std::endl;
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

  std::string VariableMapper::getAttributeValue(const xmlpp::Element* node, std::string const & attributeName){
    auto attribute = node->get_attribute(attributeName);
    if (!attribute){
      throw std::invalid_argument("Error parsing xml file. Attribute '"+attributeName + "' not found in node '" + node->get_name() +"'.");
    }
    return attribute->get_value();
  }
  
} // namespace ChimeraTK

