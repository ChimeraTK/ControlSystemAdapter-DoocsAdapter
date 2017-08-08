#include "VariableMapper.h"

#include <libxml++/libxml++.h>

namespace ChimeraTK{
  VariableMapper & VariableMapper::getInstance(){
    static VariableMapper instance;
    return instance;
  }

  void VariableMapper::prepareOutput(std::string xmlFile, std::set< std::string > inputVariables){
    xmlpp::DomParser parser;
    //    parser.set_validate();
    parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
    parser.parse_file(xmlFile);
  }

} // namespace ChimeraTK

