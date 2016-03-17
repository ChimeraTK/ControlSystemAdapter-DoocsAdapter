#include "IndependentControlCore.h"
#include <DoocsAdapter.h>

#include <iostream>

BEGIN_DOOCS_SERVER("Cosade server", 10)
   static IndependentControlCore independentControlCore(doocsAdapter.getDevicePVManager());
   std::cout << "this is eq_create" << std::endl;
END_DOOCS_SERVER()

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
