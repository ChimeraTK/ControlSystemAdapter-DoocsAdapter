#ifndef __myd_int__
#define __myd_int__


#include <boost/bind.hpp>
#include <boost/function.hpp>



class myD_int : public D_int
{
private:
	boost::function < void (int const &, int const & ) >	_onSetCallbackFunction;	// (newValue, oldValue)
	boost::function < int () >								_onGetCallbackFunction;
    
    
public:
    
    unsigned int __on_set_callback_f_call_counter;                       // __
    unsigned int __on_get_callback_f_call_counter;                       // __

    myD_int (const char *pn, EqFct *ef) :    D_int(pn, ef),
                                             __on_set_callback_f_call_counter(0),
                                             __on_get_callback_f_call_counter(0)    {}



    void setOnSetCallbackFunction (boost::function < void (int const &, int const & ) > onSetCallbackFunction)
         {
             _onSetCallbackFunction = onSetCallbackFunction;
         }


    void setOnGetCallbackFunction (boost::function < int () > onGetCallbackFunction)
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

