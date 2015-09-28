#ifndef __EMPTY_SERVER_FUNCTIONS_H__
#define __EMPTY_SERVER_FUNCTIONS_H__

/* These functions have to be defined when linking to the server library.
   These are the minimal implementations which don't do anything.
*/

/* These functions are never used in a real server, so they never can be
   called and are excluded from the coverage report.
 */
//LCOV_EXCL_START 

const char* object_name = "empty";
EqFct *eq_create(int /*eq_code*/, void*){ return NULL; }
void eq_init_epilog(){}
void eq_init_prolog(){}
void refresh_epilog(){}
void refresh_prolog(){}
void interrupt_usr1_prolog (int ) {}
void interrupt_usr1_epilog (int ) {}
void interrupt_usr2_prolog () {}
void interrupt_usr2_epilog () {}
void post_init_prolog() {}
void post_init_epilog() {}
void eq_cancel() {}

//LCOV_EXCL_STOP

#endif// __EMPTY_SERVER_FUNCTIONS_H__
