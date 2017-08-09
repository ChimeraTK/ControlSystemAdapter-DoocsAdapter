#include "VariableMapper.h"

#include <libxml++/libxml++.h>
#include <iostream>

namespace ChimeraTK{
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

  VariableMapper & VariableMapper::getInstance(){
    static VariableMapper instance;
    return instance;
  }

  void VariableMapper::processLocation(xmlpp::Node const * locationNode){
    const xmlpp::Element* location = dynamic_cast<const xmlpp::Element*>(locationNode);
    std::string name = location->get_attribute("name")->get_value();

    std::cout << "Found location: " << name << std::endl;

    for (auto const & node : location->get_children()){
        if (nodeIsWhitespace(node)) continue;
        
        if (node->get_name() == "property"){
          processProperty(node, name);
        }else if (node->get_name() == "import"){
          processLocationImport(node, name);
        }else{
          std::cout << "FIXME: Implement location node '" << node->get_name()
                    << "'! Current implementation does nothing" << std::endl;
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
      std::cout << "Whoopy, name from source is not implemented yet" << std::endl;
      name = "Whoopsy";
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
      std::cout << "Importing location: " <<  nodeAsText->get_content() << std::endl;
      // loop source tree, cut beginning, replace / with _ and add a property 
    }
  }
    
  void VariableMapper::prepareOutput(std::string xmlFile, std::set< std::string > inputVariables){
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

      std::cout << "****************************\nPredefined printout in "<< xmlFile<<":\n" << std::endl;
      print_node(rootNode);

      std::cout << "\n My interpretation for "<< xmlFile << "\n===================================" << std::endl;
      
      for (auto const & mainNode : rootNode->get_children()){
        if (nodeIsWhitespace(mainNode)) continue;
        
        if (mainNode->get_name() == "location"){
          processLocation(mainNode);
        }else{
          std::cout << "FIXME: Implement main node '" << mainNode->get_name()
                    << "'! Current implementation does nothing" << std::endl;
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

