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

bool mapCompare (std::map< std::string, std::shared_ptr< PropertyDescription > > const & lhs,
                 std::map< std::string, AutoPropertyDescription > const & rhs) {
  // No predicate needed because there is operator== for pairs already.
  if ( lhs.size() != rhs.size() ){
    std::cout << "Map size comparison failed: lhs.size() " << lhs.size() << ", rhs.size() " << rhs.size() << std::endl;// LCOV_EXCL_LINE
    return false;// LCOV_EXCL_LINE
  }
  auto r=rhs.cbegin();
  for (auto l=lhs.cbegin(); l != lhs.cend(); ++l){
    if( l->first != r->first ){
      std::cout << "PV names are not equal: " <<  l->first << ", "<< r->first << std::endl;
      return false;
    }
    auto castedLValue = std::dynamic_pointer_cast< AutoPropertyDescription >(l->second);
    if (!castedLValue){
      std::cout << "cast for " << l->first << " failed" << std::endl;      
      return false;
    }
    if ( !(r->second == (*castedLValue)) ){
      std::cout << "### check this ###" << std::endl;
      std::cout << r->second.source << " " << castedLValue->source << std::endl;
      std::cout << r->second.location << " " << castedLValue->location << std::endl;
      std::cout << r->second.name << " " << castedLValue->name << std::endl;
      std::cout << r->second.hasHistory << " " << castedLValue->hasHistory << std::endl;
      std::cout << r->second.isWriteable << " " << castedLValue->isWriteable << std::endl;
      std::cout << "Internal comparisng for " << l->first << " failed" << std::endl;
      return false;
    }
    ++r;
  }
  return true;
}

void testXmlParsing(std::string xmlFile, std::map< std::string, AutoPropertyDescription > const & propertyMap){
  VariableMapper & vm = VariableMapper::getInstance();
  vm.prepareOutput(xmlFile, generateInputVariables());
  //vm.print();
  BOOST_CHECK( mapCompare( vm.getAllProperties(), propertyMap) );
}

BOOST_AUTO_TEST_CASE( testEvaluateBool ){
  // typo/ invalid syntax
  try{
    VariableMapper::evaluateBool("fale");
    BOOST_ERROR("testEvaluateBool did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for invalid bool syntax:\n      "
              << e.what() << std::endl;
  }
  BOOST_CHECK( VariableMapper::evaluateBool("false") == false);
  BOOST_CHECK( VariableMapper::evaluateBool("False") == false);
  BOOST_CHECK( VariableMapper::evaluateBool("FALSE") == false);
  BOOST_CHECK( VariableMapper::evaluateBool("true"));
  BOOST_CHECK( VariableMapper::evaluateBool("True"));
  BOOST_CHECK( VariableMapper::evaluateBool("TRUE"));
  BOOST_CHECK( VariableMapper::evaluateBool("0") == false);
  BOOST_CHECK( VariableMapper::evaluateBool("1"));
}

BOOST_AUTO_TEST_CASE( testImportAll ){
  std::map< std::string, AutoPropertyDescription > propertyMap(
                                                  { {"/A/a/di",  {"/A/a/di", "A","a.di"}},
                                                    {"/A/a/do",  {"/A/a/do", "A","a.do"}},
                                                    {"/A/b",     {"/A/b",    "A","b"}},
                                                    {"/B/a/dr",  {"/B/a/dr", "B","a.dr"}},
                                                    {"/B/c/de",  {"/B/c/de", "B","c.de"}},
                                                    {"/B/c/gne", {"/B/c/gne","B","c.gne"}},
                                                    {"/C/a/da",  {"/C/a/da", "C","a.da"}},
                                                    {"/C/b/ge",  {"/C/b/ge", "C","b.ge"}},
                                                    {"/C/c/be",  {"/C/c/be", "C","c.be"}},
                                                    {"/C/c/de",  {"/C/c/de", "C","c.de"}},
                                                    {"/DIRECT/DOUBLE",  {"/DIRECT/DOUBLE","DIRECT","DOUBLE"}},
                                                    {"/DIRECT/DOUBLE_ARRAY",  {"/DIRECT/DOUBLE_ARRAY","DIRECT","DOUBLE_ARRAY"}},
                                                    {"/DIRECT/INT",  {"/DIRECT/INT", "DIRECT","INT"}},
                                                    {"/DIRECT/INT_ARRAY",  {"/DIRECT/INT_ARRAY","DIRECT","INT_ARRAY"}}
                                                  });
  testXmlParsing("variableTreeXml/importAll.xml", propertyMap);

  // test direct mapping without xml
  VariableMapper & vm = VariableMapper::getInstance();
  vm.clear();
  BOOST_CHECK(  vm.getAllProperties().empty() );
  vm.directImport( generateInputVariables());
  BOOST_CHECK( mapCompare( vm.getAllProperties(), propertyMap) );

  // modify the expected property map for the renaming case
  propertyMap["/DIRECT/DOUBLE"]= AutoPropertyDescription("/DIRECT/DOUBLE","DIRECT","BAR");
  propertyMap["/DIRECT/INT"]= AutoPropertyDescription("/DIRECT/INT","DIRECT","FOO");
  testXmlParsing("variableTreeXml/globalImportAndRename.xml", propertyMap);
}

BOOST_AUTO_TEST_CASE( testGetPropertiesInLocation ){
  // same input as for testAll, so we know the overall output is OK due to the separate test.
  VariableMapper & vm = VariableMapper::getInstance();
  vm.prepareOutput("variableTreeXml/importAll.xml", generateInputVariables());
  BOOST_CHECK( mapCompare( vm.getPropertiesInLocation("A"), { {"/A/a/di",  {"/A/a/di","A","a.di"}},
                                                              {"/A/a/do",  {"/A/a/do","A","a.do"}},
                                                              {"/A/b",     {"/A/b",   "A","b"}}
                                                             } ) );
  BOOST_CHECK( mapCompare( vm.getPropertiesInLocation("B"), { {"/B/a/dr",  {"/B/a/dr", "B","a.dr"}},
                                                              {"/B/c/de",  {"/B/c/de", "B","c.de"}},
                                                              {"/B/c/gne", {"/B/c/gne","B","c.gne"}}
                                                             } ) );
}

BOOST_AUTO_TEST_CASE( testImportIntoLocation ){
  std::map< std::string, AutoPropertyDescription > propertyMap(
                                                  { {"/A/a/di",  {"/A/a/di", "MASTER","A.a.di"}},
                                                    {"/A/a/do",  {"/A/a/do", "MASTER","A.a.do"}},
                                                    {"/A/b",     {"/A/b",    "MASTER","A.b"}},
                                                    {"/B/a/dr",  {"/B/a/dr", "MASTER","B.a.dr"}},
                                                    {"/B/c/de",  {"/B/c/de", "MASTER","B.c.de"}},
                                                    {"/B/c/gne", {"/B/c/gne","MASTER","B.c.gne"}},
                                                    {"/C/a/da",  {"/C/a/da", "MASTER","C.a.da"}},
                                                    {"/C/b/ge",  {"/C/b/ge", "MASTER","C.b.ge"}},
                                                    {"/C/c/be",  {"/C/c/be", "MASTER","C.c.be"}},
                                                    {"/C/c/de",  {"/C/c/de", "MASTER","C.c.de"}},
                                                    {"/DIRECT/DOUBLE",  {"/DIRECT/DOUBLE","MASTER","DIRECT.DOUBLE"}},
                                                    {"/DIRECT/DOUBLE_ARRAY",  {"/DIRECT/DOUBLE_ARRAY","MASTER","DIRECT.DOUBLE_ARRAY"}},
                                                    {"/DIRECT/INT",  {"/DIRECT/INT","MASTER","DIRECT.INT"}},
                                                    {"/DIRECT/INT_ARRAY",  {"/DIRECT/INT_ARRAY","MASTER","DIRECT.INT_ARRAY"}}
                                                  });
  testXmlParsing("variableTreeXml/importAllIntoLocation.xml", propertyMap);
}

BOOST_AUTO_TEST_CASE( testImportWithDirectory ){
  std::map< std::string, AutoPropertyDescription > propertyMap(
                                                  { {"/A/a/di",  {"/A/a/di", "MASTER","myStuff.a.di"}},
                                                    {"/A/a/do",  {"/A/a/do", "MASTER","myStuff.a.do"}},
                                                    {"/A/b",     {"/A/b",    "MASTER","myStuff.b"}},
                                                    {"/B/a/dr",  {"/B/a/dr", "DOT_REMOVER","stuffWithDot.a.dr"}},
                                                    {"/B/c/de",  {"/B/c/de", "DOT_REMOVER","stuffWithDot.c.de"}},
                                                    {"/B/c/gne", {"/B/c/gne","DOT_REMOVER","stuffWithDot.c.gne"}}
                                                  });
  testXmlParsing("variableTreeXml/importWithDirectory.xml", propertyMap);
}

BOOST_AUTO_TEST_CASE( testWrongGlobalDirectory ){
  // directory is not allowed in global imports, only when importing inside a location
  try{
    testXmlParsing("variableTreeXml/wrongGlobalDirectory.xml", {});
    BOOST_ERROR("testWrongGlobalDirectory did not throw as expected.");// LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for directory in global import:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testImportTooShort ){
  try{
    testXmlParsing("variableTreeXml/globalImportPartTooShort.xml", {});
    BOOST_ERROR("testImportTooShort did not throw as expected.");// LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for too short tree depth:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testGlobalImportPart ){
  testXmlParsing("variableTreeXml/globalImportPart.xml", { {"/B/a/dr",  {"/B/a/dr", "a","dr"}},
                                                           {"/B/c/de",  {"/B/c/de", "c","de"}},
                                                           {"/B/c/gne", {"/B/c/gne","c","gne"}} });
}

BOOST_AUTO_TEST_CASE( testImportAndRename ){
  testXmlParsing("variableTreeXml/importAndRename.xml", { {"/DIRECT/DOUBLE",  {"/DIRECT/DOUBLE","DIRECT","BAR"}},
                                                          {"/DIRECT/DOUBLE_ARRAY",  {"/DIRECT/DOUBLE_ARRAY","DIRECT","DOUBLE_ARRAY"}},
                                                          {"/DIRECT/INT",  {"/DIRECT/INT","DIRECT","FOO"}},
                                                          {"/DIRECT/INT_ARRAY",  {"/DIRECT/INT_ARRAY","DIRECT","INT_ARRAY"}}
                                                         });
}

BOOST_AUTO_TEST_CASE( testCherryPicking ){
  testXmlParsing("variableTreeXml/cherryPick.xml", { {"/A/b/do",  {"/A/b/do", "DIRECT","A.b.do"}},
                                                     {"/B/c/de",  {"/B/c/de", "B","c.de"}},
                                                     {"/DIRECT/INT",  {"/DIRECT/INT","DIRECT","INT"}}
                                                   });
}

BOOST_AUTO_TEST_CASE( testDuplicateSource ){
  try{
    testXmlParsing("variableTreeXml/duplicateSource.xml", {});
    BOOST_ERROR("testDuplicateSource did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for duplicate sources:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testUnknownMainNode ){
  try{
    testXmlParsing("variableTreeXml/unknownMainNode.xml", {});
    BOOST_ERROR("testUnknownMainNode did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for unknown main node:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testUnkownLocationNode ){
  try{
    testXmlParsing("variableTreeXml/unknownLocationNode.xml", {});
    BOOST_ERROR("testUnknownLocationNode did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for unknown location node:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testLocationTurnOffOn ){
  testXmlParsing("variableTreeXml/locationTurnOffOn.xml", { {"/A/a/di",  {"/A/a/di","DUMMY_LOCATION","a.di",false}},
                                                            {"/A/a/do",  {"/A/a/do","DUMMY_LOCATION","A.a.do",true}},
                                                            {"/A/b",     {"/A/b","DUMMY_LOCATION","b",false}},
                                                            {"/B/a/dr",  {"/B/a/dr","ANOTHER_LOCATION","a.dr",true, false}},
                                                            {"/B/c/de",  {"/B/c/de","ANOTHER_LOCATION","c.de",true, false}},
	                                                    {"/B/c/gne", {"/B/c/gne","ANOTHER_LOCATION","WRITE_ME"}},
                                                       });
}

BOOST_AUTO_TEST_CASE( testGlobalTurnOffOnHistory ){
  testXmlParsing("variableTreeXml/globalTurnOnOffHistory.xml",
                 { {"/A/a/di",  {"/A/a/di","DUMMY_LOCATION","a.di",true}},
                   {"/A/a/do",  {"/A/a/do","DUMMY_LOCATION","A.a.do",false}},
                   {"/A/b",     {"/A/b",   "DUMMY_LOCATION","b",true}},
                   {"/B/a/dr",  {"/B/a/dr","ANOTHER_LOCATION","a.dr",false}},
                   {"/B/c/de",  {"/B/c/de","ANOTHER_LOCATION","c.de",false}},
                   {"/B/c/gne", {"/B/c/gne","ANOTHER_LOCATION","DONT_WRITE_ME",false,false}},
                   {"/C/a/da",  {"/C/a/da","NO_LOCATION_MODIFIERS","a.da",true}},
                   {"/C/b/ge",  {"/C/b/ge","NO_LOCATION_MODIFIERS","b.ge",false}},
                   {"/C/c/be",  {"/C/c/be","NO_LOCATION_MODIFIERS","c.be",false}},
                   {"/C/c/de",  {"/C/c/de","NO_LOCATION_MODIFIERS","c.de",false}},
                   {"/DIRECT/DOUBLE",  {"/DIRECT/DOUBLE","DIRECT","DOUBLE",false}},
                   {"/DIRECT/DOUBLE_ARRAY",  {"/DIRECT/DOUBLE_ARRAY","DIRECT","DOUBLE_ARRAY",false}},
                   {"/DIRECT/INT",  {"/DIRECT/INT","DIRECT","INT",false}},
                   {"/DIRECT/INT_ARRAY",  {"/DIRECT/INT_ARRAY","DIRECT","INT_ARRAY",false}}
                 });
}

BOOST_AUTO_TEST_CASE( testGlobalTurnOffOnWriteable ){
  testXmlParsing("variableTreeXml/globalTurnOnOffWriteable.xml",
                 { {"/A/a/di",  {"/A/a/di","DUMMY_LOCATION","a.di",true,false}},
                   {"/A/a/do",  {"/A/a/do","DUMMY_LOCATION","A.a.do",false,false}},
                   {"/A/b",     {"/A/b",   "DUMMY_LOCATION","b",true,false}},
                   {"/B/a/dr",  {"/B/a/dr","ANOTHER_LOCATION","a.dr",true, true}},
                   {"/B/c/de",  {"/B/c/de","ANOTHER_LOCATION","c.de",true, true}},
                   {"/B/c/gne", {"/B/c/gne","ANOTHER_LOCATION","DONT_WRITE_ME",true,false}},
                   {"/C/a/da",  {"/C/a/da","NO_LOCATION_MODIFIERS","a.da",true,false}},
                   {"/C/b/ge",  {"/C/b/ge","NO_LOCATION_MODIFIERS","b.ge",true,true}},
                   {"/C/c/be",  {"/C/c/be","NO_LOCATION_MODIFIERS","c.be",true,false}},
                   {"/C/c/de",  {"/C/c/de","NO_LOCATION_MODIFIERS","c.de",true,false}},
                   {"/DIRECT/DOUBLE",  {"/DIRECT/DOUBLE","DIRECT","DOUBLE",true,false}},
                   {"/DIRECT/DOUBLE_ARRAY",  {"/DIRECT/DOUBLE_ARRAY","DIRECT","DOUBLE_ARRAY",true,false}},
                   {"/DIRECT/INT",  {"/DIRECT/INT","DIRECT","INT",true,false}},
                   {"/DIRECT/INT_ARRAY",  {"/DIRECT/INT_ARRAY","DIRECT","INT_ARRAY",true,false}}
                 });
}

