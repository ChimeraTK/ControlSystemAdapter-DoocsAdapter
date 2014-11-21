#ifndef __myd_float__
#define __myd_float__



#include "myD_xxx.hpp"


class myD_float : public myD_xxx<float>, public D_float
{
    
public:
    
    myD_float (const char *pn, EqFct *ef) :    D_float(pn, ef) {}


            // accessors with callbacks (based on standard DOOCS property interface)
	void	set_value (float val)
            {
                D_float::set_value(val);
                if (_onSetCallbackFunction)
                {
                    _onSetCallbackFunction (val, val);
                }
            }
	float	value ()
            {
                if (_onGetCallbackFunction)
                {
                    float ongetcallbackresult = _onGetCallbackFunction ();
                    // doing sth based on ongetcallbackresult ???
                    ongetcallbackresult += 2; // suppress -Wunused-variable
                }
                return D_float::value();
            }


            // accessors without callbacks (extension of the original DOOCS property interface)
	void	set_value_without_callback (float val)
            {
                D_float::set_value(val);
            }
	float	value_without_callback ()
            {
                return D_float::value();
            }
};






#endif /* __myd_float__ */

