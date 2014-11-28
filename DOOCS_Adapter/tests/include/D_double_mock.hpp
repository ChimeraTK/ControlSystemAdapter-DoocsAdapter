#ifndef __d_double_mock__
#define __d_double_mock__


#include "eq_fct.h"



/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_double
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_double::set_value
 */
class D_double
{

protected:
    const char          * pn_; // suppress -Wunused-parameter
	const EqFct         * ef_; // suppress -Wunused-parameter
    
	double				value_;
    
public:

    D_double (const char *pn, EqFct *ef) : pn_(pn), ef_(ef), value_(0) {}
    D_double (const char *pn)            : pn_(pn),          value_(0) {}


	virtual void	set_value (double val)
					{
						value_ = val;
					}

	virtual double	value ()
					{
						return value_;
					}
};







#endif /* __d_double_mock__ */

