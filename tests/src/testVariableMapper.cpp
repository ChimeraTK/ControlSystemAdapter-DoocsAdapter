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
  if ( lhs.size() != rhs.size() ){
    std::cout << "Map size comparison failed: lhs.size() " << lhs.size() << ", rhs.size() " << rhs.size() << std::endl;// LCOV_EXCL_LINE
    return false;// LCOV_EXCL_LINE
  }
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
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
  testXmlParsing("variableTreeXml/importLocation.xml", { {"/A/a/di",  {"B","a.di"}},
                                                         {"/A/a/do",  {"B","a.do"}},
                                                         {"/A/b",     {"B","b"}}
                                                       });
}

BOOST_AUTO_TEST_CASE( testImportAll ){
  std::map< std::string, VariableMapper::PropertyDescription > propertyMap(
                                                  { {"/A/a/di",  {"A","a.di"}},
                                                    {"/A/a/do",  {"A","a.do"}},
                                                    {"/A/b",     {"A","b"}},
                                                    {"/B/a/dr",  {"B","a.dr"}},
                                                    {"/B/c/de",  {"B","c.de"}},
                                                    {"/B/c/gne", {"B","c.gne"}},
                                                    {"/C/a/da",  {"C","a.da"}},
                                                    {"/C/b/ge",  {"C","b.ge"}},
                                                    {"/C/c/be",  {"C","c.be"}},
                                                    {"/C/c/de",  {"C","c.de"}},
                                                    {"/DIRECT/DOUBLE",  {"DIRECT","DOUBLE"}},
                                                    {"/DIRECT/DOUBLE_ARRAY",  {"DIRECT","DOUBLE_ARRAY"}},
                                                    {"/DIRECT/INT",  {"DIRECT","INT"}},
                                                    {"/DIRECT/INT_ARRAY",  {"DIRECT","INT_ARRAY"}}
                                                  });
  testXmlParsing("variableTreeXml/importAll.xml", propertyMap);

  // test direct mapping without xml
  VariableMapper & vm = VariableMapper::getInstance();
  vm.clear();
  BOOST_CHECK(  vm.getAllProperties().empty() );
  vm.directImport( generateInputVariables());
  BOOST_CHECK( mapCompare( vm.getAllProperties(), propertyMap) );

  // modify the expected property map for the renaming case
  propertyMap["/DIRECT/DOUBLE"]= VariableMapper::PropertyDescription("DIRECT","BAR");
  propertyMap["/DIRECT/INT"]= VariableMapper::PropertyDescription("DIRECT","FOO");
  testXmlParsing("variableTreeXml/globalImportAndRename.xml", propertyMap);
}

BOOST_AUTO_TEST_CASE( testGetPropertiesInLocation ){
  // same input as for testAll, so we know the overall output is OK due to the separate test.
  VariableMapper & vm = VariableMapper::getInstance();
  vm.prepareOutput("variableTreeXml/importAll.xml", generateInputVariables());
  BOOST_CHECK( mapCompare( vm.getPropertiesInLocation("A"), { {"/A/a/di",  {"A","a.di"}},
                                                              {"/A/a/do",  {"A","a.do"}},
                                                              {"/A/b",     {"A","b"}}
                                                             } ) );
  BOOST_CHECK( mapCompare( vm.getPropertiesInLocation("B"), { {"/B/a/dr",  {"B","a.dr"}},
                                                              {"/B/c/de",  {"B","c.de"}},
                                                              {"/B/c/gne",     {"B","c.gne"}}
                                                             } ) );
}

BOOST_AUTO_TEST_CASE( testImportIntoLocation ){
  std::map< std::string, VariableMapper::PropertyDescription > propertyMap(
                                                  { {"/A/a/di",  {"MASTER","A.a.di"}},
                                                    {"/A/a/do",  {"MASTER","A.a.do"}},
                                                    {"/A/b",     {"MASTER","A.b"}},
                                                    {"/B/a/dr",  {"MASTER","B.a.dr"}},
                                                    {"/B/c/de",  {"MASTER","B.c.de"}},
                                                    {"/B/c/gne", {"MASTER","B.c.gne"}},
                                                    {"/C/a/da",  {"MASTER","C.a.da"}},
                                                    {"/C/b/ge",  {"MASTER","C.b.ge"}},
                                                    {"/C/c/be",  {"MASTER","C.c.be"}},
                                                    {"/C/c/de",  {"MASTER","C.c.de"}},
                                                      {"/DIRECT/DOUBLE",  {"MASTER","DIRECT.DOUBLE"}},
                                                    {"/DIRECT/DOUBLE_ARRAY",  {"MASTER","DIRECT.DOUBLE_ARRAY"}},
                                                    {"/DIRECT/INT",  {"MASTER","DIRECT.INT"}},
                                                    {"/DIRECT/INT_ARRAY",  {"MASTER","DIRECT.INT_ARRAY"}}
                                                  });
  testXmlParsing("variableTreeXml/importAllIntoLocation.xml", propertyMap);
}

BOOST_AUTO_TEST_CASE( testImportTooShort ){
  try{
    testXmlParsing("variableTreeXml/globalImportPartTooShort.xml", {});
    BOOST_ERROR("testImportTooShort did not throw as expected.");// LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << "For manually checking the exception message for too short tree depth:\n"
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testGlobalImportPart ){
  testXmlParsing("variableTreeXml/globalImportPart.xml", { {"/B/a/dr",  {"a","dr"}},
                                                           {"/B/c/de",  {"c","de"}},
                                                           {"/B/c/gne", {"c","gne"}} });
}

BOOST_AUTO_TEST_CASE( testImportAndRename ){
  testXmlParsing("variableTreeXml/importAndRename.xml", { {"/DIRECT/DOUBLE",  {"DIRECT","BAR"}},
                                                          {"/DIRECT/DOUBLE_ARRAY",  {"DIRECT","DOUBLE_ARRAY"}},
                                                          {"/DIRECT/INT",  {"DIRECT","FOO"}},
                                                          {"/DIRECT/INT_ARRAY",  {"DIRECT","INT_ARRAY"}}
                                                         });
}

BOOST_AUTO_TEST_CASE( testCherryPicking ){
  testXmlParsing("variableTreeXml/cherryPick.xml", { {"/A/b/do",  {"DIRECT","A.b.do"}},
                                                     {"/B/c/de",  {"B","c.de"}},
                                                     {"/DIRECT/INT",  {"DIRECT","INT"}}
                                                   });
}

BOOST_AUTO_TEST_CASE( testDuplicateSource ){
  try{
    testXmlParsing("variableTreeXml/duplicateSource.xml", {});
    BOOST_ERROR("testDuplicateSource did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << "For manually checking the exception message for duplicate sources:\n"
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testUnknownMainNode ){
  try{
    testXmlParsing("variableTreeXml/unknownMainNode.xml", {});
    BOOST_ERROR("testUnknownMainNode did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << "For manually checking the exception message for unknown main node:\n"
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testUnkownLocationNode ){
  try{
    testXmlParsing("variableTreeXml/unknownLocationNode.xml", {});
    BOOST_ERROR("testUnknownLocationNode did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << "For manually checking the exception message for unknown location node:\n"
              << e.what() << std::endl;
  }
}
