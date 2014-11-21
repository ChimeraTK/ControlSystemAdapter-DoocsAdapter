#ifndef __myd_int__
#define __myd_int__



#include "myD_xxx.hpp"


class myD_int : public myD_xxx<int>, public D_int
{
    
public:
    
    myD_int (const char *pn, EqFct *ef) :    D_int(pn, ef) {}


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






#endif /* __myd_int__ */

