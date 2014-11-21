#ifndef __d_float_mock__
#define __d_float_mock__





class EqFct {}; // stub


/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_float
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_float::set_value
 */
class D_float
{

protected:
    const char          * pn_; // suppress -Wunused-parameter
	const EqFct         * ef_; // suppress -Wunused-parameter
    
	float				value_;
    
public:

    D_float (const char *pn, EqFct *ef) : pn_(pn), ef_(ef), value_(0) {}
    D_float (const char *pn)            : pn_(pn),          value_(0) {}


	virtual void	set_value (float val)
					{
						value_ = val;
					}

	virtual float	value ()
					{
						return value_;
					}
};







#endif /* __d_float_mock__ */

