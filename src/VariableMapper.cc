#include "VariableMapper.h"

#include <libxml++/libxml++.h>
#include <iostream>
#include <regex>
#include "splitStringAtFirstSlash.h"

namespace ChimeraTK{
//LCOV_EXCL_START
void print_indentation(unsigned int indentation)
{
  for(unsigned int i = 0; i < indentation; ++i)
    std::cout << " ";
}

  bool nodeIsWhitespace(const xmlpp::Node* node){
    const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
    if(nodeAsText){
      return nodeAsText->is_white_space();
    }
    return false;
  }
  
void print_node(const xmlpp::Node* node, unsigned int indentation = 0)
{
  std::cout << std::endl; //Separate nodes by an empty line.
  
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if(nodeText && nodeText->is_white_space()){ //Let's ignore the indenting - you don't always want to do this.
    std::cout << "--ignoring whitespace" << std::endl;
    return;
  }
    
  Glib::ustring nodename = node->get_name();

  if(!nodeText && !nodeComment && !nodename.empty()) //Let's not say "name: text".
  {
    print_indentation(indentation);
    std::cout << "Node name = " << node->get_name() << std::endl;
    std::cout << "Node name = " << nodename << std::endl;
  }
  else if(nodeText) //Let's say when it's text. - e.g. let's say what that white space is.
  {
    print_indentation(indentation);
    std::cout << "Text Node" << std::endl;
  }

  //Treat the various node types differently: 
  if(nodeText)
  {
    print_indentation(indentation);
    std::cout << "text = \"" << nodeText->get_content() << "\"" << std::endl;
  }
  else if(nodeComment)
  {
    print_indentation(indentation);
    std::cout << "comment = " << nodeComment->get_content() << std::endl;
  }
  else if(nodeContent)
  {
    print_indentation(indentation);
    std::cout << "content = " << nodeContent->get_content() << std::endl;
  }
  else if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node))
  {
    //A normal Element node:

    //line() works only for ElementNodes.
    print_indentation(indentation);
    std::cout << "     line = " << node->get_line() << std::endl;

    //Print attributes:
    const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
    for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
    {
      const xmlpp::Attribute* attribute = *iter;
      print_indentation(indentation);
      std::cout << "  Attribute " << attribute->get_name() << " = " << attribute->get_value() << std::endl;
    }

    const xmlpp::Attribute* attribute = nodeElement->get_attribute("title");
    if(attribute)
    {
      std::cout << "title found: =" << attribute->get_value() << std::endl;
      
    }
  }
  
  if(!nodeContent)
  {
    //Recurse through child nodes:
    xmlpp::Node::NodeList list = node->get_children();
    for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
    {
      print_node(*iter, indentation + 2); //recursive
    }
  }
}
//LCOV_EXCL_STOP
  
  VariableMapper & VariableMapper::getInstance(){
    static VariableMapper instance;
    return instance;
  }

  void VariableMapper::processLocation(xmlpp::Node const * locationNode){
    const xmlpp::Element* location = dynamic_cast<const xmlpp::Element*>(locationNode);
    std::string locationName = location->get_attribute("name")->get_value();

    std::cout << "Found location: " << locationName << std::endl;

    for (auto const & node : location->get_children()){
        if (nodeIsWhitespace(node)) continue;
        
        if (node->get_name() == "property"){
          processProperty(node, locationName);
        }else if (node->get_name() == "import"){
          processLocationImport(node, locationName);
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
    std::cout << "going to create property: " << locationName << "/" << name
              << " from " << absoluteSource << std::endl;
    auto existingCandidate = _inputSortedDescriptions.find(absoluteSource);
    if (existingCandidate != _inputSortedDescriptions.end()){
      auto existingPropertyDescription = existingCandidate->second;
      throw std::logic_error(std::string("Invalid XML content for ") + absoluteSource + " -> "+ locationName+"/" +name +". Process variable already defined to point to " + existingPropertyDescription.location + "/" + existingPropertyDescription.name);
    }else{
      _inputSortedDescriptions[absoluteSource] = PropertyDescription(locationName, name);
    }
   
  }

  void VariableMapper::processLocationImport(xmlpp::Node const * importNode, std::string locationName){
    for (auto const & node : importNode->get_children()){
      const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
      std::string importSource = nodeAsText->get_content();
      std::cout << "Importing in location '"<<locationName <<"': " <<  importSource << std::endl;

      // a slash will be added after the source, so we make the source empty for an import of everything
      if (importSource == "/"){
        importSource = "";
      }
     
      // loop source tree, cut beginning, replace / with _ and add a property
      for (auto const & processVariable : _inputVariables){
        if (_inputSortedDescriptions.find(processVariable) != _inputSortedDescriptions.end()){
          std::cout << processVariable << " alread in the map. Not importing" << std::endl;
          continue;
        }
        
        if ( processVariable.find( importSource+"/") == 0 ){
          // processVariable starts with wanted source
          std::cout << "importing " << processVariable << " from " <<  importSource << " into "
                    << locationName << std::endl;
          auto propertyNameSource = processVariable.substr( importSource.size() + 1); // add the slash to be removed
          std::cout << "propertyNameSource " << propertyNameSource << std::endl;
          auto propertyName = std::regex_replace(propertyNameSource, std::regex("/"), ".");
          std::cout << "new property name is " << propertyName << std::endl;
          _inputSortedDescriptions[processVariable] = PropertyDescription(locationName, propertyName);
        }
      }
    }
  }

  void VariableMapper::processGlobalImport(xmlpp::Node const * importNode){
    for (auto const & node : importNode->get_children()){
      const xmlpp::TextNode* nodeAsText = dynamic_cast<const xmlpp::TextNode*>(node);
      std::string importSource = nodeAsText->get_content();
      std::cout << "Globaly importing in : " <<  importSource << std::endl;

      // a slash will be added, so we make the source empty for a global import of everything
      if (importSource == "/"){
        importSource = "";
      }
      
      for (auto const & processVariable : _inputVariables){
        if (_inputSortedDescriptions.find(processVariable) != _inputSortedDescriptions.end()){
          std::cout << processVariable << " alread in the map. Not importing" << std::endl;
          continue;
        }

        if ( processVariable.find( importSource+"/") == 0 ){
          std::cout << "about to import " << processVariable << std::endl;
          // This variable is to be imported
          auto nameSource = processVariable.substr( importSource.size() + 1); // add the slash to be removed
          auto locationAndPropertyName = splitStringAtFirstSlash(nameSource);
          auto locationName = locationAndPropertyName.first;
          auto propertyName = locationAndPropertyName.second;
          if (locationName.empty() ){
            throw std::logic_error(std::string("Invalid XML content in global import of ") + importSource + ":  Cannot create location name from '" + nameSource + "', one hirarchy level is missing.");
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

      //      std::cout << "****************************\nPredefined printout in "<< xmlFile<<":\n" << std::endl;
      //      print_node(rootNode);

      std::cout << "\n My interpretation for "<< xmlFile << "\n===================================" << std::endl;
      
      for (auto const & mainNode : rootNode->get_children()){
        if (nodeIsWhitespace(mainNode)) continue;
        
        if (mainNode->get_name() == "location"){
          processLocation(mainNode);
        }else if (mainNode->get_name() == "import"){
          processGlobalImport(mainNode);
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

} // namespace ChimeraTK

