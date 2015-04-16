// file eq_dpvatestsrv.h
//
// Test Eq functions class
//
// This is a general test server. The server get the name of the 
// test file and for the history file at compile-time as define parameter
// inside the makefile
//
//
// Olaf Hensler, - MCS4 -
//
// last update:
// 	07. Feb. 2010
//
#ifndef eq_dpvatestsrv_h
#define eq_dpvatestsrv_h

#include	"eq_fct.h"

#define CodeDpvatestsrv 10	// eq_fct_type number for the .conf file

class EqFctDpvatestsrv;	// forward declaration of the following class

// Declaration of the overloaded class 
class D_myfloat : public D_float { 

public:

    D_myfloat(char* pn, EqFctDpvatestsrv* efp) : D_float(pn, (EqFct*)efp) {}

    void set_value(float f);	// change set_value() method
};


class EqFctDpvatestsrv : public EqFct {

private:
	D_floathist	value_;
	D_int		int_value_;
	D_status	status_;
	D_bit		bit0_;
	D_bit		bit1_;
	D_myfloat	overload_;

public:
	EqFctDpvatestsrv  ( );
	~EqFctDpvatestsrv ( ) {}

	void	init ();	// started after creation of all Eq's
	
	void	update ();	// called frequently by the internal timer

	int	fct_code()	{ return CodeDpvatestsrv; }
};

#endif
