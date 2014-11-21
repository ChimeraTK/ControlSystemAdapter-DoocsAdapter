#ifndef __myd_double__
#define __myd_double__



#include "myD_xxx.hpp"


class myD_double : public myD_xxx<double>, public D_double
{
    
public:
    
    myD_double (const char *pn, EqFct *ef) :    D_double(pn, ef) {}


            // accessors with callbacks (based on standard DOOCS property interface)
	void	set_value (double val)
            {
                D_double::set_value(val);
                if (_onSetCallbackFunction)
                {
                    _onSetCallbackFunction (val, val);
                }
            }
	double	value ()
            {
                if (_onGetCallbackFunction)
                {
                    double ongetcallbackresult = _onGetCallbackFunction ();
                    // doing sth based on ongetcallbackresult ???
                    ongetcallbackresult += 2; // suppress -Wunused-variable
                }
                return D_double::value();
            }


            // accessors without callbacks (extension of the original DOOCS property interface)
	void	set_value_without_callback (double val)
            {
                D_double::set_value(val);
            }
	double	value_without_callback ()
            {
                return D_double::value();
            }
};






#endif /* __myd_double__ */

