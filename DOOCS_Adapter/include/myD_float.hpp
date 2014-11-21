#ifndef __myd_float__
#define __myd_float__


#include <boost/bind.hpp>
#include <boost/function.hpp>



class myD_float : public D_float
{
private:
	boost::function < void (float const &, float const & ) >	_onSetCallbackFunction;	// (newValue, oldValue)
	boost::function < float () >								_onGetCallbackFunction;
    
    
public:
    
    myD_float (const char *pn, EqFct *ef) :    D_float(pn, ef) {}



    void setOnSetCallbackFunction (boost::function < void (float const &, float const & ) > onSetCallbackFunction)
         {
             _onSetCallbackFunction = onSetCallbackFunction;
         }


    void setOnGetCallbackFunction (boost::function < float () > onGetCallbackFunction)
         {
             _onGetCallbackFunction = onGetCallbackFunction;
         }


    void clearOnSetCallbackFunction ()
         {
             _onSetCallbackFunction.clear ();
         }


    void clearOnGetCallbackFunction ()
         {
             _onGetCallbackFunction.clear ();
         }



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

