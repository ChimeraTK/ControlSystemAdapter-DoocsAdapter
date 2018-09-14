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

void testXmlParsing(std::string xmlFile){
  VariableMapper & vm = VariableMapper::getInstance();
  vm.prepareOutput(xmlFile, generateInputVariables());
  //vm.print();
  // only used for error handling. nothing tested here
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

BOOST_AUTO_TEST_CASE( testWrongGlobalDirectory ){
  // directory is not allowed in global imports, only when importing inside a location
  try{
    testXmlParsing("variableTreeXml/wrongGlobalDirectory.xml");
    BOOST_ERROR("testWrongGlobalDirectory did not throw as expected.");// LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for directory in global import:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testImportTooShort ){
  try{
    testXmlParsing("variableTreeXml/globalImportPartTooShort.xml");
    BOOST_ERROR("testImportTooShort did not throw as expected.");// LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for too short tree depth:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testUnknownMainNode ){
  try{
    testXmlParsing("variableTreeXml/unknownMainNode.xml");
    BOOST_ERROR("testUnknownMainNode did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for unknown main node:\n      "
              << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( testUnkownLocationNode ){
  try{
    testXmlParsing("variableTreeXml/unknownLocationNode.xml");
    BOOST_ERROR("testUnknownLocationNode did not throw as expected"); // LCOV_EXCL_LINE
  }catch(std::logic_error & e){
    std::cout << " -- For manually checking the exception message for unknown location node:\n      "
              << e.what() << std::endl;
  }
}

