#include "CosadeEqFct.h"

const char * object_name = "Cosade server";

// The usual function to create locations. Present in every doocs server.
EqFct * eq_create (int eq_code, void *){
   switch (eq_code) {
      case CODE_COSADE:
	 return  new CosadeEqFct ();
      default:
	 return (EqFct *) 0;
   }
}

// all the bloat we have to implement for DOOCS although we don't need it
void eq_init_prolog() {}
void eq_init_epilog() {}
void refresh_prolog() {}
void refresh_epilog() {}
void interrupt_usr1_prolog(int) {}
void interrupt_usr2_prolog(void) {}
void interrupt_usr1_epilog(int) {}
void interrupt_usr2_epilog(void) {}
void post_init_prolog(void) {}
void post_init_epilog(void) {}
void eq_cancel(void) {}
