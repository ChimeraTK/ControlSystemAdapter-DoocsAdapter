#include "VariableMapper.h"

#include "splitStringAtFirstSlash.h"
#include <ChimeraTK/RegisterPath.h>
#include <algorithm>
#include <iostream>
#include <libxml++/libxml++.h>
#include <locale>
#include <regex>

namespace ChimeraTK {

  /********************************************************************************************************************/

  VariableMapper& VariableMapper::getInstance() {
    static VariableMapper instance;
    return instance;
  }

  /********************************************************************************************************************/

  bool VariableMapper::nodeIsWhitespace(const xmlpp::Node* node) {
    const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
    if(nodeAsText) {
      return nodeAsText->is_white_space();
    }
    return false;
  }

  /********************************************************************************************************************/

  void VariableMapper::processLocationNode(xmlpp::Node const* locationNode) {
    const xmlpp::Element* location = dynamic_cast<const xmlpp::Element*>(locationNode);
    if(!location) {
      throw std::invalid_argument("Error parsing xml file in location node.");
    }
    std::string locationName = getAttributeValue(location, "name");

    processCode(location, locationName);

    for(auto const& node : location->get_children()) {
      if(nodeIsWhitespace(node)) continue;
      if(dynamic_cast<xmlpp::CommentNode const*>(node)) continue;

      if(node->get_name() == "property") {
        processNode(node, locationName);
      }
      else if(node->get_name() == "import") {
        processImportNode(node, locationName);
      }
      else if(node->get_name() == "has_history") {
        auto& locationInfo = _locationDefaults[locationName];
        locationInfo.useHasHistoryDefault = true;
        locationInfo.hasHistory = evaluateBool(getContentString(node));
      }
      else if(node->get_name() == "persist") {
        auto& locationInfo = _locationDefaults[locationName];
        locationInfo.usePersistDefault = true;
        locationInfo.persist = PersistConfig(getContentString(node));
      }
      else if(node->get_name() == "is_writeable") {
        auto& locationInfo = _locationDefaults[locationName];
        locationInfo.useIsWriteableDefault = true;
        locationInfo.isWriteable = evaluateBool(getContentString(node));
      }
      else if(node->get_name() == "macro_pulse_number_source") {
        auto& locationInfo = _locationDefaults[locationName];
        locationInfo.useMacroPulseNumberSourceDefault = true;
        locationInfo.macroPulseNumberSource = getContentString(node);
      }
      else if(node->get_name() == "data_matching") {
        auto& locationInfo = _locationDefaults[locationName];
        locationInfo.useDataMatchingDefault = true;
        locationInfo.dataMatching = evaluateDataMatching(getContentString(node));
      }
      else if(node->get_name() == "D_spectrum") {
        processSpectrumNode(node, locationName);
      }
      else if(node->get_name() == "D_array") {
        processNode(node, locationName);
      }
      else if(node->get_name() == "D_xy") {
        processXyNode(node, locationName);
      }
      else if(node->get_name() == "D_ifff") {
        processIfffNode(node, locationName);
      }
      else if(node->get_name() == "set_error") {
        processSetErrorNode(node, locationName);
      }
      else {
        throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName +
            ": Unknown node '" + node->get_name() + "'");
      }
    }
  }

  /********************************************************************************************************************/

  std::string determineName(const xmlpp::Element* property, std::string source) {
    const xmlpp::Attribute* nameAttribute = property->get_attribute("name");
    // if a name is given in xml take it
    if(nameAttribute) {
      return nameAttribute->get_value();
    }
    else { // auto-determine the name from the source
      ///@todo FIXME: use register path to do the slash and replace fiddeling
      std::string name;
      if(source[0] == '/') {
        name = source.substr(1);
      }
      else {
        name = source;
      }
      // replace / with . in name
      return std::regex_replace(name, std::regex("/"), ".");
    }
  }

  /********************************************************************************************************************/

  std::string getAbsoluteSource(std::string source, std::string locationName) {
    if(source[0] == '/') {
      return source;
    }
    else {
      return std::string("/") + locationName + "/" + source;
    }
  }

  /********************************************************************************************************************/

  template<class PROPERTY_DESCRIPTION_TYPE>
  void VariableMapper::processHistoryAndWritableAttributes(PROPERTY_DESCRIPTION_TYPE propertyDescription,
      const xmlpp::Element* propertyXmlElement, std::string locationName) {
    auto hasHistoryNodes = propertyXmlElement->get_children("has_history");
    if(!hasHistoryNodes.empty()) {
      propertyDescription->hasHistory = evaluateBool(getContentString(hasHistoryNodes.front()));
    }
    else {
      propertyDescription->hasHistory = getHasHistoryDefault(locationName);
    }

    auto persistNodes = propertyXmlElement->get_children("persist");
    if(!persistNodes.empty()) {
      propertyDescription->persist = PersistConfig(getContentString(persistNodes.front()));
    }
    else {
      propertyDescription->persist = _locationDefaults[locationName].persist;
    }

    auto isWriteableNodes = propertyXmlElement->get_children("is_writeable");
    if(!isWriteableNodes.empty()) {
      propertyDescription->isWriteable = evaluateBool(getContentString(isWriteableNodes.front()));
    }
    else {
      propertyDescription->isWriteable = getIsWriteableDefault(locationName);
    }

    auto publishZeroMQ = propertyXmlElement->get_children("publish_ZMQ");
    if(!publishZeroMQ.empty()) {
      propertyDescription->publishZMQ = evaluateBool(getContentString(publishZeroMQ.front()));
    }
    else {
      propertyDescription->publishZMQ = false;
    }

    auto macroPulseNumberSource = propertyXmlElement->get_children("macro_pulse_number_source");
    if(!macroPulseNumberSource.empty()) {
      propertyDescription->macroPulseNumberSource = getContentString(macroPulseNumberSource.front());
    }
    else {
      propertyDescription->macroPulseNumberSource = getMacroPusleNumberSourceDefault(locationName);
    }

    auto dataMatching = propertyXmlElement->get_children("data_matching");
    if(!dataMatching.empty()) {
      propertyDescription->dataMatching = evaluateDataMatching(getContentString(dataMatching.front()));
    }
    else {
      propertyDescription->dataMatching = getDataMatchingDefault(locationName);
    }
  }

  /********************************************************************************************************************/

  void VariableMapper::addDescription(
      std::shared_ptr<PropertyDescription> const& propertyDescription, std::list<std::string> const& absoluteSources) {
    _descriptions.push_back(propertyDescription);
    for(auto& source : absoluteSources) {
      _usedInputVariables.insert(source);
    }
  }

  /********************************************************************************************************************/

  xmlpp::Element const* asXmlElement(xmlpp::Node const* node) {
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
    if(!element) {
      throw std::invalid_argument("Error parsing xml file: Node is not an element node: " + node->get_name());
    }
    return element;
  }

  /********************************************************************************************************************/

  void VariableMapper::processNode(xmlpp::Node const* propertyNode, std::string locationName) {
    auto property = asXmlElement(propertyNode);

    std::string source = getAttributeValue(property, "source");
    std::string name = determineName(property, source);

    const xmlpp::Attribute* typeAttribute = property->get_attribute("type");
    std::map<std::string, AutoPropertyDescription::DataType> dataTypeMap(
        {{"auto", AutoPropertyDescription::DataType::Auto}, {"byte", AutoPropertyDescription::DataType::Byte},
            {"short", AutoPropertyDescription::DataType::Short}, {"int", AutoPropertyDescription::DataType::Int},
            {"long", AutoPropertyDescription::DataType::Long}, {"float", AutoPropertyDescription::DataType::Float},
            {"double", AutoPropertyDescription::DataType::Double}, {"bool", AutoPropertyDescription::DataType::Bool}});

    auto type = AutoPropertyDescription::DataType::Auto;

    if(typeAttribute) {
      try {
        type = dataTypeMap.at(typeAttribute->get_value());
      }
      catch(std::out_of_range&) {
        throw ChimeraTK::logic_error("Unknown type attribute in line " + std::to_string(property->get_line()) + ": " +
            typeAttribute->get_value());
      }
    }

    std::string absoluteSource = getAbsoluteSource(source, locationName);

    // prepare the property description
    auto autoPropertyDescription = std::make_shared<AutoPropertyDescription>(absoluteSource, locationName, name, type);

    processHistoryAndWritableAttributes(autoPropertyDescription, property, locationName);

    addDescription(autoPropertyDescription, {absoluteSource});
  }

  /********************************************************************************************************************/

  void VariableMapper::processSpectrumNode(xmlpp::Node const* node, std::string locationName) {
    auto spectrumXml = asXmlElement(node);

    // the "main source" of a spectum
    std::string source = getAttributeValue(spectrumXml, "source");
    std::string name = determineName(spectrumXml, source);
    std::string absoluteSource = getAbsoluteSource(source, locationName);

    // prepare the property description
    auto spectrumDescription = std::make_shared<SpectrumDescription>(absoluteSource, locationName, name);

    processHistoryAndWritableAttributes(spectrumDescription, spectrumXml, locationName);

    auto startNodes = spectrumXml->get_children("start");
    if(!startNodes.empty()) {
      spectrumDescription->start = std::stof(getContentString(startNodes.front()));
    }
    std::list<std::string> usedVariables({absoluteSource});
    auto incrementNodes = spectrumXml->get_children("increment");
    if(!incrementNodes.empty()) {
      spectrumDescription->increment = std::stof(getContentString(incrementNodes.front()));
    }
    auto startSourceNodes = spectrumXml->get_children("startSource");
    if(!startSourceNodes.empty()) {
      auto startSource = getContentString(startSourceNodes.front());
      spectrumDescription->startSource = startSource;
      usedVariables.push_back(startSource);
    }
    auto incrementSourceNodes = spectrumXml->get_children("incrementSource");
    if(!incrementSourceNodes.empty()) {
      auto incrementSource = getContentString(incrementSourceNodes.front());
      spectrumDescription->incrementSource = incrementSource;
      usedVariables.push_back(incrementSource);
    }
    auto numberOfBuffersNodes = spectrumXml->get_children("numberOfBuffers");
    if(!numberOfBuffersNodes.empty()) {
      auto numberOfBuffers = getContentString(numberOfBuffersNodes.front());
      spectrumDescription->numberOfBuffers = std::stoi(numberOfBuffers);
      usedVariables.push_back(numberOfBuffers);
    }

    auto descriptionNode = spectrumXml->get_first_child("description");
    if(descriptionNode != nullptr) spectrumDescription->description = getContentString(descriptionNode);

    auto unitNodes = spectrumXml->get_children("unit");
    for(const auto unit : unitNodes) {
      auto unitElement = asXmlElement(unit);
      auto axis = getAttributeValue(unitElement, "axis");
      if(axis != "x" && axis != "y")
        throw std::invalid_argument("Unsupported axis in D_spectrum, must be \"x\" or \"y\": " + axis);

      std::string label;
      if(not unit->get_children().empty()) label = getContentString(unit);

      spectrumDescription->axis[axis].label = label;
      try {
        spectrumDescription->axis[axis].logarithmic = std::stoi(getAttributeValue(unitElement, "logarithmic"));
      }
      catch(std::invalid_argument&) {
      }

      try {
        spectrumDescription->axis[axis].start = std::stof(getAttributeValue(unitElement, "start"));
      }
      catch(std::invalid_argument&) {
      }

      try {
        spectrumDescription->axis[axis].stop = std::stof(getAttributeValue(unitElement, "stop"));
      }
      catch(std::invalid_argument&) {
      }
    }
    addDescription(spectrumDescription, usedVariables);
  }

  /********************************************************************************************************************/

  void VariableMapper::processXyNode(xmlpp::Node const* node, std::string& locationName) {
    auto xyXml = asXmlElement(node);

    auto xSource = getAttributeValue(xyXml, "x_source");
    auto ySource = getAttributeValue(xyXml, "y_source");
    auto name = determineName(xyXml, xSource + "_" + ySource);
    auto xAbsoluteSource = getAbsoluteSource(xSource, locationName);
    auto yAbsoluteSource = getAbsoluteSource(ySource, locationName);

    auto xyDescription = std::make_shared<XyDescription>(xAbsoluteSource, yAbsoluteSource, locationName, name, false);
    processHistoryAndWritableAttributes(xyDescription, xyXml, locationName);

    auto descriptionNode = xyXml->get_first_child("description");
    if(descriptionNode != nullptr) xyDescription->description = getContentString(descriptionNode);

    auto unitNodes = xyXml->get_children("unit");
    for(const auto unit : unitNodes) {
      auto unitElement = asXmlElement(unit);
      auto axis = getAttributeValue(unitElement, "axis");
      if(axis != "x" && axis != "y")
        throw std::invalid_argument("Unsupported axis in D_xy, must be \"x\" or \"y\": " + axis);

      std::string label;
      if(not unit->get_children().empty()) label = getContentString(unit);

      xyDescription->axis[axis].label = label;
      try {
        xyDescription->axis[axis].logarithmic = std::stoi(getAttributeValue(unitElement, "logarithmic"));
      }
      catch(std::invalid_argument&) {
      }

      try {
        xyDescription->axis[axis].start = std::stof(getAttributeValue(unitElement, "start"));
      }
      catch(std::invalid_argument&) {
      }

      try {
        xyDescription->axis[axis].stop = std::stof(getAttributeValue(unitElement, "stop"));
      }
      catch(std::invalid_argument&) {
      }
    }

    addDescription(xyDescription, {xAbsoluteSource, yAbsoluteSource});
  }
  /********************************************************************************************************************/

  void VariableMapper::processIfffNode(xmlpp::Node const* node, std::string& locationName) {
    auto ifffXml = asXmlElement(node);

    auto i1Source = getAttributeValue(ifffXml, "i1_source");
    auto f1Source = getAttributeValue(ifffXml, "f1_source");
    auto f2Source = getAttributeValue(ifffXml, "f2_source");
    auto f3Source = getAttributeValue(ifffXml, "f3_source");
    auto name = getAttributeValue(ifffXml, "name");
    std::list<std::string> absoluteSources;
    absoluteSources.push_back(getAbsoluteSource(i1Source, locationName));
    absoluteSources.push_back(getAbsoluteSource(f1Source, locationName));
    absoluteSources.push_back(getAbsoluteSource(f2Source, locationName));
    absoluteSources.push_back(getAbsoluteSource(f3Source, locationName));

    auto ifffDescription =
        std::make_shared<IfffDescription>(i1Source, f1Source, f2Source, f3Source, locationName, name);
    processHistoryAndWritableAttributes(ifffDescription, ifffXml, locationName);

    addDescription(ifffDescription, absoluteSources);
  }

  /********************************************************************************************************************/
  void VariableMapper::processSetErrorNode(xmlpp::Node const* node, std::string& locationName) {
    for(auto const& errRepInfo : _errorReportingInfos) {
      if(errRepInfo.targetLocation == locationName)
        throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName +
            ": tag <set_error> is allowed only once per location.");
    }
    ErrorReportingInfo errInfo;
    errInfo.targetLocation = locationName;

    auto setErrorXml = asXmlElement(node);
    std::string s = getAttributeValue(setErrorXml, "statusCodeSource");
    errInfo.statusCodeSource = s;
    // matching status message source is found automatically by naming convention - if it exists
    errInfo.statusStringSource = s + "_message";
    _errorReportingInfos.push_back(errInfo);
  }

  /********************************************************************************************************************/

  void VariableMapper::processImportNode(xmlpp::Node const* importNode, std::string importLocationName) {
    const xmlpp::Element* importElement = dynamic_cast<const xmlpp::Element*>(importNode);
    std::string directory;
    if(importElement) {
      // look for a directory attribute
      const xmlpp::Attribute* directoryAttribute = importElement->get_attribute("directory");
      if(directoryAttribute) {
        directory = directoryAttribute->get_value();
      }
    }

    for(auto const& node : importNode->get_children()) {
      const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
      std::string importSource = nodeAsText->get_content();
      import(importSource, importLocationName, directory);
    }
  }

  /********************************************************************************************************************/

  void VariableMapper::import(std::string importSource, std::string importLocationName, std::string directory) {
    // a slash will be added after the source, so we make the source empty for an
    // import of everything
    if(importSource == "/") {
      importSource = "";
    }

    // loop source tree, cut beginning, replace / with _ and add a property
    for(auto const& processVariable : _inputVariables) {
      if(_usedInputVariables.find(processVariable) != _usedInputVariables.end()) {
        continue;
      }

      if(processVariable.find(importSource + "/") == 0) {
        // processVariable starts with wanted source
        auto nameSource = processVariable.substr(importSource.size() + 1); // add the slash to be removed
        // we use the register path because it removes duplicate separators and
        // allows to use . as separater to replace all / with .
        ChimeraTK::RegisterPath propertyName;
        std::string locationName;
        if(importLocationName.empty()) {
          // a global import, try to get the location name from the source
          auto locationAndPropertyName = splitStringAtFirstSlash(nameSource);
          locationName = locationAndPropertyName.first;
          propertyName = locationAndPropertyName.second;
          if(locationName.empty()) {
            throw std::logic_error(std::string("Invalid XML content in global import of ") +
                (importSource.empty() ? "/" : importSource) + ":  Cannot create location name from '" + nameSource +
                "', one hirarchy level is missing.");
          }
          // convenience for the user: You get an error message is you try a
          // global import with directory (in case you did not validate your xml
          // against the schema).
          if(!directory.empty()) {
            throw std::logic_error(std::string("Invalid XML content in global import of ") +
                (importSource.empty() ? "/" : importSource) + ":  You cannot have a directory in a global import.");
          }
        }
        else {
          // import into a location, we know the location name.
          // add the directory first, then add the name source property
          propertyName /= directory;
          propertyName /= nameSource;
          locationName = importLocationName;
        }

        // get the property name with . instead of /
        propertyName.setAltSeparator(".");
        auto autoPropertyDescription = std::make_shared<AutoPropertyDescription>(
            processVariable, locationName, propertyName.getWithAltSeparator());

        // we are importing, so all properties get the intended defaults (not
        // individual settings)
        autoPropertyDescription->hasHistory = getHasHistoryDefault(locationName);
        autoPropertyDescription->isWriteable = getIsWriteableDefault(locationName);
        autoPropertyDescription->persist = getPersistDefault(locationName);
        autoPropertyDescription->macroPulseNumberSource = getMacroPusleNumberSourceDefault(locationName);
        autoPropertyDescription->dataMatching = getDataMatchingDefault(locationName);

        addDescription(autoPropertyDescription, {processVariable});
      }
    }
  }

  /********************************************************************************************************************/

  void VariableMapper::processCode(xmlpp::Element const* location, std::string locationName) {
    // If the the code is set in the location tag of the mapping xml file,
    // this function adds the location and fct_code to a map _inputLocationAndCode.
    // It throws an exeption if the code is not an integer. The code must be greater than 1,
    // because 1 is reserved for the server. And the code must be consistent.
    auto locationCodeAttribute = location->get_attribute("code");
    if(locationCodeAttribute) {
      int locationCode = 0;
      try {
        locationCode = std::stoi(locationCodeAttribute->get_value());
      }
      catch(std::invalid_argument&) {
        throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName +
            ": Unknown code '" + locationCodeAttribute->get_value() + "'");
      }
      if(locationCode < 2) {
        throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName + ": code '" +
            locationCodeAttribute->get_value() + "' must be > 1, in doocs 1 is reserved for server");
      }

      auto result = _inputLocationAndCode.insert(std::pair<std::string, int>(locationName, locationCode));
      if(result.second == false) {                 //test if pair is already in map
        if(result.first->second != locationCode) { //and test if code is the same like before
          // maybe an exection is too much and a warning is enough?
          throw std::invalid_argument(std::string("Error parsing xml file in location ") + locationName + ": code '" +
              std::to_string(locationCode) + "' must be consistent with location code before '" +
              std::to_string(result.first->second) + "'");
        }
      }
    }
  }

  /********************************************************************************************************************/

  void VariableMapper::prepareOutput(std::string xmlFile, std::set<std::string> inputVariables) {
    clear();
    _inputVariables = inputVariables;

    xmlpp::DomParser parser;
    //    parser.set_validate();
    parser.set_substitute_entities(); // We just want the text to be
                                      // resolved/unescaped automatically.
    parser.parse_file(xmlFile);

    if(parser) {
      // Walk the tree:
      const xmlpp::Node* rootNode = parser.get_document()->get_root_node(); // deleted by DomParser.

      for(auto const& mainNode : rootNode->get_children()) {
        if(nodeIsWhitespace(mainNode)) continue;
        if(dynamic_cast<xmlpp::CommentNode const*>(mainNode)) continue;

        if(mainNode->get_name() == "location") {
          processLocationNode(mainNode);
        }
        else if(mainNode->get_name() == "import") {
          processImportNode(mainNode);
        }
        else if(mainNode->get_name() == "has_history") {
          _globalDefaults.hasHistory = evaluateBool(getContentString(mainNode));
        }
        else if(mainNode->get_name() == "is_writeable") {
          _globalDefaults.isWriteable = evaluateBool(getContentString(mainNode));
        }
        else if(mainNode->get_name() == "persist") {
          _globalDefaults.persist = PersistConfig(getContentString(mainNode));
        }
        else if(mainNode->get_name() == "macro_pulse_number_source") {
          _globalDefaults.macroPulseNumberSource = getContentString(mainNode);
        }
        else if(mainNode->get_name() == "data_matching") {
          _globalDefaults.dataMatching = evaluateDataMatching(getContentString(mainNode));
        }
        else {
          throw std::invalid_argument(
              std::string("Error parsing xml file ") + xmlFile + ": Unknown node '" + mainNode->get_name() + "'");
        }
      }
    }
    else {
      throw std::invalid_argument(std::string("Error parsing xml file ") + xmlFile + ". No document produced.");
    }
  }

  /********************************************************************************************************************/

  std::list<std::shared_ptr<PropertyDescription>> const& VariableMapper::getAllProperties() const {
    return _descriptions;
  }

  /********************************************************************************************************************/

  std::list<std::shared_ptr<PropertyDescription>> VariableMapper::getPropertiesInLocation(std::string location) const {
    std::list<std::shared_ptr<PropertyDescription>> output;

    for(auto const& variable : _descriptions) {
      if(variable->location == location) {
        (void)output.push_back(variable);
      }
    }
    return output;
  }

  /********************************************************************************************************************/

  std::unordered_set<std::string> VariableMapper::getAllLocations() const {
    std::unordered_set<std::string> result;
    for(auto const& variable : _descriptions) {
      result.insert(variable->location);
    }
    return result;
  }

  /********************************************************************************************************************/

  void VariableMapper::directImport(std::set<std::string> inputVariables) {
    clear();
    _inputVariables = inputVariables;
    import("/",
        ""); // import from /, create location names from first level of the tree
  }

  /********************************************************************************************************************/

  void VariableMapper::clear() {
    _inputVariables.clear();
    _usedInputVariables.clear();
    _locationDefaults.clear();
    _globalDefaults = PropertyAttributes();
    _descriptions.clear();
  }

  /********************************************************************************************************************/

  /// printing the map is useful for debugging
  void VariableMapper::print(std::ostream& os) const {
    os << "====== VariableMapper =====" << std::endl;
    for(auto& description : _descriptions) {
      description->print(os);
    }
    os << "======= Mapping End =======" << std::endl;
  }

  /********************************************************************************************************************/

  bool VariableMapper::evaluateBool(std::string txt) {
    transform(txt.begin(), txt.end(), txt.begin(), ::tolower);
    if(txt == "false" or txt == "0") {
      return false;
    }
    else if(txt == "true" or txt == "1") {
      return true;
    }
    else {
      throw std::invalid_argument(std::string("Error parsing xml file: could not convert to bool: ") + txt);
    }
  }

  /********************************************************************************************************************/

  DataConsistencyGroup::MatchingMode VariableMapper::evaluateDataMatching(std::string txt) {
    DataConsistencyGroup::MatchingMode dataMatching;
    if(txt == "exact") {
      dataMatching = DataConsistencyGroup::MatchingMode::exact;
    }
    else if(txt == "none") {
      dataMatching = DataConsistencyGroup::MatchingMode::none;
    }
    else {
      throw std::invalid_argument("Unknown data matching mode specified in xml file: " + txt);
    }
    return dataMatching;
  }

  /*******************************************************************************************************************/

  std::string VariableMapper::getContentString(xmlpp::Node const* node) {
    for(auto const& subNode : node->get_children()) {
      const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(subNode);
      if(nodeAsText) {
        if(nodeAsText->is_white_space()) {
          continue;
        }
        return nodeAsText->get_content();
      }
    }
    // No TextNode found: empty string
    return "";
  }

  /********************************************************************************************************************/

  bool VariableMapper::getHasHistoryDefault(std::string const& locationName) {
    // if there is no default setting for the location, we will get the "default
    // default" which has useHasHistoryDefault disabled which is auto-generated by
    // the [] operator
    auto locationInfo = _locationDefaults[locationName];
    if(locationInfo.useHasHistoryDefault) {
      return locationInfo.hasHistory;
    }
    else {
      return _globalDefaults.hasHistory;
    }
  }

  /********************************************************************************************************************/

  bool VariableMapper::getIsWriteableDefault(std::string const& locationName) {
    // if there is no default setting for the location, we will get the "default
    // default" which has useIsWriteableDefault disabled which is auto-generated
    // by the [] operator
    auto locationInfo = _locationDefaults[locationName];
    if(locationInfo.useIsWriteableDefault) {
      return locationInfo.isWriteable;
    }
    else {
      return _globalDefaults.isWriteable;
    }
  }

  PersistConfig VariableMapper::getPersistDefault(std::string const& locationName) {
    auto locationInfo = _locationDefaults[locationName];
    if(locationInfo.usePersistDefault) {
      return locationInfo.persist;
    }
    else {
      return _globalDefaults.persist;
    }
  }

  /********************************************************************************************************************/

  std::string VariableMapper::getMacroPusleNumberSourceDefault(std::string const& locationName) {
    // if there is no default setting for the location, we will get the "default
    // default" which has useMacroPulseNumberSourceDefault disabled which is
    // auto-generated by the [] operator
    auto locationInfo = _locationDefaults[locationName];
    if(locationInfo.useMacroPulseNumberSourceDefault) {
      return locationInfo.macroPulseNumberSource;
    }
    else {
      return _globalDefaults.macroPulseNumberSource;
    }
  }
  /********************************************************************************************************************/

  DataConsistencyGroup::MatchingMode VariableMapper::getDataMatchingDefault(std::string const& locationName) {
    // if there is no default setting for the location, we will get the "default
    // default" which has useMacroPulseNumberSourceDefault disabled which is
    // auto-generated by the [] operator
    auto locationInfo = _locationDefaults[locationName];
    if(locationInfo.useDataMatchingDefault) {
      return locationInfo.dataMatching;
    }
    else {
      return _globalDefaults.dataMatching;
    }
  }

  /********************************************************************************************************************/

  std::string VariableMapper::getAttributeValue(const xmlpp::Element* node, std::string const& attributeName) {
    auto attribute = node->get_attribute(attributeName);
    if(!attribute) {
      throw std::invalid_argument(
          "Error parsing xml file. Attribute '" + attributeName + "' not found in node '" + node->get_name() + "'.");
    }
    return attribute->get_value();
  }

} // namespace ChimeraTK
