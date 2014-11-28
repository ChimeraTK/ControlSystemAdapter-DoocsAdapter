#ifndef __m4uD_float__
#define __m4uD_float__



#include "m4uD_xxx.hpp"


class m4uD_float : public m4uD_xxx<float>, public D_float
{
    
public:
    
    m4uD_float (const char *pn, EqFct *ef) :    D_float(pn, ef) {}


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






#endif /* __m4uD_float__ */

