#define BOOST_TEST_MODULE DoocsProcessArrayTest
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
//#include <boost/test/test_case_template.hpp>

#include "VariableMapper.h"
using namespace ChimeraTK;

BOOST_AUTO_TEST_CASE( testCreation ){
  VariableMapper & vm = VariableMapper::getInstance();
  VariableMapper & vm2 = VariableMapper::getInstance();
  BOOST_CHECK( &vm == &vm2 );
}

/** Here is the example tree on which the examples and tests are based:
 *\verbatim
/
├── A
│   ├── a
│   │   ├── di
│   │   └── do
│   └── b
├── B
│   ├── a
│   │   └── dr
│   └── c
│       ├── de
│       └── gne
├── C
│   ├── a
│   │   └── da
│   ├── b
│   │   └── ge
│   └── c
│       ├── be
│       └── de
└── DIRECT
    ├── DOUBLE
    ├── DOUBLE_ARRAY
    ├── INT
    └── INT_ARRAY
 *\endverbatim
 */
std::set< std::string > generateInputVariables(){
  std::set< std::string > inputVariables;
  inputVariables.insert("/A/a/di");
  inputVariables.insert("/A/a/do");
  inputVariables.insert("/A/b");
  inputVariables.insert("/B/a/dr");
  inputVariables.insert("/B/c/de");
  inputVariables.insert("/B/c/gne");
  inputVariables.insert("/C/a/da");  
  inputVariables.insert("/C/b/ge");  
  inputVariables.insert("/C/c/be");  
  inputVariables.insert("/C/c/de"); // there also is a c/de in B
  inputVariables.insert("/DIRECT/DOUBLE");  
  inputVariables.insert("/DIRECT/DOUBLE_ARRAY");  
  inputVariables.insert("/DIRECT/INT");  
  inputVariables.insert("/DIRECT/INT_ARRAY");

  return inputVariables;
}

template <typename Map>
bool mapCompare (Map const &lhs, Map const &rhs) {
  // No predicate needed because there is operator== for pairs already.
  std::cout << "lhs.size() " << lhs.size() << ", rhs.size() " << rhs.size() << std::endl;
  return lhs.size() == rhs.size()
    && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

void testXmlParsing(std::string xmlFile, std::map< std::string, VariableMapper::PropertyDescription > propertyMap){
  VariableMapper & vm = VariableMapper::getInstance();
  vm.prepareOutput(xmlFile, generateInputVariables());
  BOOST_CHECK( mapCompare( vm.getAllProperties(), propertyMap) );
}

BOOST_AUTO_TEST_CASE( testRename ){
  testXmlParsing("variableTreeXml/rename.xml", { {"/A/b/do",{"DIRECT","LOLO"}},
                                                 {"/DIRECT/INT",{"DIRECT","TEMP"}}
                                               } );
}

BOOST_AUTO_TEST_CASE( testImportLocation ){
  testXmlParsing("variableTreeXml/importLocation.xml", { {"/A/a/di",  {"B","a_di"}},
                                                         {"/A/a/do",  {"B","a_do"}},
                                                         {"/A/b",     {"B","b"}}
                                                       });
}

BOOST_AUTO_TEST_CASE( testImportAll ){
  testXmlParsing("variableTreeXml/importAll.xml", { {"/A/a/di",  {"A","a_di"}},
                                                    {"/A/a/do",  {"A","a_do"}},
                                                    {"/A/b",     {"A","b"}},
                                                    {"/B/a/dr",  {"B","a_dr"}},
                                                    {"/B/c/de",  {"B","c_de"}},
                                                    {"/B/c/gne", {"B","c_gne"}},
                                                    {"/C/a/da",  {"C","a_da"}},
                                                    {"/C/b/ge",  {"C","b_ge"}},
                                                    {"/C/c/be",  {"C","c_be"}},
                                                    {"/C/c/de",  {"C","c_de"}},
                                                    {"/DIRECT/DOUBLE",  {"DIRECT","DOUBLE"}},
                                                    {"/DIRECT/DOUBLE",  {"DIRECT","DOUBLE_ARRAY"}},
                                                    {"/DIRECT/INT",  {"DIRECT","INT"}},
                                                    {"/DIRECT/INT",  {"DIRECT","INT_ARRAY"}}
                                                  });
}

BOOST_AUTO_TEST_CASE( testImportAndRename ){
  testXmlParsing("variableTreeXml/importAndRename.xml", { {"/DIRECT/DOUBLE",  {"DIRECT","BAR"}},
                                                          {"/DIRECT/DOUBLE_ARRAY",  {"DIRECT","DOUBLE_ARRAY"}},
                                                          {"/DIRECT/INT",  {"DIRECT","FOO"}},
                                                          {"/DIRECT/INT_ARRAY",  {"DIRECT","INT_ARRAY"}}
                                                         });
}
