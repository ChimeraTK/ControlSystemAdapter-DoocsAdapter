#include "TestScalarsEqFct.h"

TestScalarsEqFct::TestScalarsEqFct() : EqFct("NAME Test Scalars EqeFct"),
				       myDoocsFloat("MY_DOOCS_FLOAT float with adapter",
						    this, NULL){
  printtostderr(name_str(), "TestScalarsEqFct constructor");
}

TestScalarsEqFct::~TestScalarsEqFct(){
  printtostderr(name_str(), "TestScalarsEqFct destructor");
}

void TestScalarsEqFct::init(){
  printtostderr(name_str(), "TestScalarsEqFct::init()");
}

void TestScalarsEqFct::update(){
  printtostderr(name_str(), "TestScalarsEqFct::update()");
}
