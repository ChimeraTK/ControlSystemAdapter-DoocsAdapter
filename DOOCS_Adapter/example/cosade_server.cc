#include "IndependentControlCore.h"
#include <CSAdapterEqFct.h>
#include <DoocsAdapter.h>

#include <iostream>

const char * object_name = "Cosade server";

// The usual function to create locations. Present in every doocs server.
// Each location is identified by a code, which is defined here.
EqFct * eq_create (int eq_code, void *){
   static mtca4u::DoocsAdapter doocsAdapter;
   static IndependentControlCore independentControlCore(doocsAdapter.getDevicePVManager());
   std::cout << "this is eq_create" << std::endl;
   switch (eq_code) {
      case 10:
	 return new mtca4u::CSAdapterEqFct("NAME = cosade", eq_code, doocsAdapter.getControlSystemPVManager());
      default:
	 return NULL;
   }
}

// all the bloat we have to implement for DOOCS although we don't need it
void eq_init_prolog() {std::cout << "this is eq_init_prolog" << std::endl;}
void eq_init_epilog() {std::cout << "this is eq_init_epilog" << std::endl;}
void refresh_prolog() {}
void refresh_epilog() {}
void interrupt_usr1_prolog(int) {}
void interrupt_usr2_prolog(void) {}
void interrupt_usr1_epilog(int) {}
void interrupt_usr2_epilog(void) {}
void post_init_prolog(void) {}
void post_init_epilog(void) {}
void eq_cancel(void) {}
