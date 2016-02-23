#include "TestScalarsEqFct.h"

/// Necessary global object name (used in error messages)
const char* object_name = "test_scalars_server";

EqFct *eq_create(int eq_code, void*)
{
    switch (eq_code)
    {
    case TestScalarsEqFct::code:
        return new TestScalarsEqFct();
    default:
        printftostderr("eq_create()", "Unknown EqFct code %d\n", eq_code);
	return NULL;
    }
}

// Called at end of init of all EqFct's
void eq_init_epilog(){}

// Called before init of all EqFct's
void eq_init_prolog(){
  // Select advanced archiver
  //    set_arch_mode(1);
}

// called after "update"
void refresh_epilog(){}

// Called before "update"
void refresh_prolog(){}

// External interrupt functions
void interrupt_usr1_prolog (int ) {}
void interrupt_usr1_epilog (int ) {}
void interrupt_usr2_prolog () {}
void interrupt_usr2_epilog () {}

// Various
void post_init_prolog() {}
void post_init_epilog() {}
void eq_cancel() {}
