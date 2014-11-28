#ifndef __d_int_mock__
#define __d_int_mock__


#include "eq_fct.h"



/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_int
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_int::set_value
 */
class D_int
{

protected:
    const char          * pn_; // suppress -Wunused-parameter
	const EqFct         * ef_; // suppress -Wunused-parameter
    
	int				value_;
    
public:
    
    unsigned int __set_value_f_call_counter;                // __
    unsigned int __value_f_call_counter;                    // __

    D_int (const char *pn, EqFct *ef) : pn_(pn), ef_(ef), value_(0), __set_value_f_call_counter(0), __value_f_call_counter(0) {}
    D_int (const char *pn)            : pn_(pn),          value_(0), __set_value_f_call_counter(0), __value_f_call_counter(0) {}


	virtual void	set_value (int val)
					{
                        __set_value_f_call_counter++;       // __
						value_ = val;
					}

	virtual int		value ()
					{
                        __value_f_call_counter++;           // __
						return value_;
					}
};







#endif /* __d_int_mock__ */

