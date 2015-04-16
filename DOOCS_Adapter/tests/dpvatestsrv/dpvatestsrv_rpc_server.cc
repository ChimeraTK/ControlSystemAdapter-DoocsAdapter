//
// file eq_dpvatestsrv.cc
//
// Dpvatestsrv Eq function class
//
// This is a general dpvatestsrv server used for the documentation 
// "DOOCSserver for Dummies"
//
// Olaf Hensler, - MCS4 -
//
// last update:
// 	07. Feb. 2010
//
//
// =====================================================================================
#include	"eq_dpvatestsrv.h"
#include	"eq_sts_codes.h"
#include	"eq_fct_errors.h"
// =====================================================================================
char*	object_name = "DpvatestsrvServer"; 
// =====================================================================================
EqFctDpvatestsrv::EqFctDpvatestsrv ( ) :	
	EqFct		("NAME = location" ),
	value_		("FLOATDATA test value with history", this, "TEST" ),
	int_value_	("INTDATA integer test value", this ),
	status_		("REGISTER a hardware refister 16bits", this ),
	bit0_		("BIT0 first bit of REGISTER", 0, &status_.stat_, this),
	bit1_		("BIT1 second bit of REGISTER", 1, &status_.stat_, this),
	overload_	("OVERLOADED_FLOAT demonstrate D_fct overloading", this)
{
}
// =====================================================================================
void	eq_init_prolog () {	// called once before init of all EqFct's
   printftostderr("eq_init_prolog","Built on host %s by %s on %s at %s",
	HOST, "O.Hensler", __DATE__, __TIME__ );
}
// =====================================================================================
//
// The init() method is call for every location during startup of the server
// Initialization of the hardware may be done here
//
void	EqFctDpvatestsrv::init ( )
{
	std::cout << "Inside init(), Location name : " << name_.value() << std::endl;
}
// =====================================================================================
void	eq_init_epilog () {}	// called once at end of init of all EqFct's
// =====================================================================================
//
// used during startup of the server to create the locations
//
EqFct *
eq_create (int eq_code, void *)
{
   EqFct* eqn;
   switch (eq_code) {
      case CodeDpvatestsrv:
         eqn =  new EqFctDpvatestsrv ();
         break;
      default:
         eqn = (EqFct *) 0;
         break;
   }
   return eqn;
}
// =====================================================================================
void refresh_prolog ()		// called before "update"
{
}
// =====================================================================================
//
// This "update" method usually does the real work in a DOOCS server
// "update" is called frequently with the rate configured by SVR.RATE
// This method is called for every location, e.g. runs in a loop over all locations
// configured inside the .conf file
//
void	EqFctDpvatestsrv::update ( )
{
 float fdata;
 int   error = 0;

   if( g_sts_.online() ) {
   	
	//
	// do some hardware readout, e.g. SEDAC, Ethernet ...
	//
	error = 0;
	
	
	if( !error ) {
	
	  std::string test;
		test = "10457.878";
		fdata = atof (test.c_str() );
		value_.set_value( fdata );	// does archiving automatically
		
		int idata = (int) atof (test.c_str() );
		int_value_.set_value( idata );
		
   		set_error( no_error );
	}
	else {
   		fdata = value_.value();
 		value_.set_value( fdata, sts_err );	// history with error flag
  		set_error( device_error );		// set error for this location
	}	
   }
   else {
   
   	fdata = value_.value();
 	value_.set_value( fdata, sts_offline );	// history with offline flag
   	set_error( offline );
   }
}
// =====================================================================================
void refresh_epilog ()	{}	// called after "update"
// =====================================================================================
//
// The following methods are provided, when the server needs to use SIGUSR1 or SIGUSR2 
// interrupts (from timing system)
//
void interrupt_usr1_prolog(int)  {}
void interrupt_usr2_prolog(void) {}
void interrupt_usr1_epilog(int)  {}
void interrupt_usr2_epilog(void) {}
void post_init_prolog(void)  	 {}
void post_init_epilog(void)	 {}
void eq_cancel(void)	 	 {}
// =====================================================================================
// =====================================================================================
// Overloaded method
void D_myfloat::set_value(float f) {
    D_float::set_value(f);    // Set the local value
    
    //
    // Do your hardware access here
    //
    std::cout << "write " << f << " to hardware here" << std::endl;
}
// =====================================================================================
