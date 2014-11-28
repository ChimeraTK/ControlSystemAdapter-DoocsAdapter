#ifndef __m4uD_int__
#define __m4uD_int__



#include "m4uD_xxx.hpp"


class m4uD_int : public m4uD_xxx<int>, public D_int
{
    
public:
    
    m4uD_int (const char *pn, EqFct *ef) :    D_int(pn, ef) {}


            // accessors with callbacks (based on standard DOOCS property interface)
	void	set_value (int val)
            {
                D_int::set_value(val);
                if (_onSetCallbackFunction)
                {
                    _onSetCallbackFunction (val, val);
                }
            }
	int		value ()
            {
                if (_onGetCallbackFunction)
                {
                    int ongetcallbackresult = _onGetCallbackFunction ();
                    // doing sth based on ongetcallbackresult ???
                    ongetcallbackresult += 2; // suppress -Wunused-variable
                }
                return D_int::value();
            }


            // accessors without callbacks (extension of the original DOOCS property interface)
	void	set_value_without_callback (int val)
            {
                D_int::set_value(val);
            }
	int		value_without_callback ()
            {
                return D_int::value();
            }
};






#endif /* __m4uD_int__ */

