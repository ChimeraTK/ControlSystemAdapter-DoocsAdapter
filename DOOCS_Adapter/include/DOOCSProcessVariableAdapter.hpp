#ifndef __doocs_process_variable_adapter__
#define __doocs_process_variable_adapter__

#include <boost/function.hpp>




class DOOCSPVAdapter
{
	
	int _t;
	
	boost::function < void (int const &, int const & ) >	_onSetCallbackFunction;	// (newValue, oldValue)
	boost::function < int () >								_onGetCallbackFunction;


public:

	DOOCSPVAdapter () : _t(0) {};

	void	setOnSetCallbackFunction ( boost::function < void (int const & , int const & ) > onSetCallbackFunction )
			{
				_onSetCallbackFunction = onSetCallbackFunction;
			}

	void	setOnGetCallbackFunction (boost::function < int () > onGetCallbackFunction)
			{
				_onGetCallbackFunction = onGetCallbackFunction;
			}
	
	void	clearOnSetCallbackFunction ()
			{
				_onSetCallbackFunction.clear ();
			}
	void	clearOnGetCallbackFunction ()
			{
				_onGetCallbackFunction.clear ();
			}
	
	void	set (int const &t)
			{
				int oldValue = _t;
				_t = t;
				if (_onSetCallbackFunction)
				{
					_onSetCallbackFunction (t, oldValue);
				}
			}
	void	setWithoutCallback (int const &t)
			{
				_t = t;
			}

	int		get ()
			{
				if (_onGetCallbackFunction)
				{
					_t = _onGetCallbackFunction ();
				}
				return _t;
			}
	int		getWithoutCallback () const
			{
				return _t;
			}


	virtual ~ DOOCSPVAdapter () {};

};






class EqFct {}; // stub


/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_int
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_int::set_value
 */
class D_int
{

protected:
	int				value_;
	
public:
    
    unsigned int __set_value_f_call_counter;                // __
    unsigned int __value_f_call_counter;                    // __

    D_int (const char *pn, EqFct *ef) : value_(0), __set_value_f_call_counter(0), __value_f_call_counter(0) {}
    D_int (const char *pn)            : value_(0), __set_value_f_call_counter(0), __value_f_call_counter(0) {}


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





class myD_int : public D_int
{
    
protected:
    DOOCSPVAdapter * doocs_adapter;     // DOOCS process variable adapter handler
	
public:
    
    unsigned int __on_set_callback_f_call_counter;                       // __
    unsigned int __on_get_callback_f_call_counter;                       // __

    myD_int (const char *pn, EqFct *ef, DOOCSPVAdapter * _da) :  doocs_adapter(_da),
                                                                 D_int(pn, ef),
                                                                 __on_set_callback_f_call_counter(0),
                                                                 __on_get_callback_f_call_counter(0)
    {
        doocs_adapter->setOnSetCallbackFunction(boost::bind (&myD_int::on_set_callback, this, _1));     // here want only to do a set. original set_value() does exactly this also
    }

	virtual void	set_value (int val)
					{
						D_int::set_value(val);
                        doocs_adapter->setWithoutCallback(val);
					}
	virtual int		value ()
					{
                        // anything else needed here?
                        // I guess not, if the values are to be remembered by both the PVA and the DOOCS type
						return D_int::value();
					}


                    // callbacks
	void    	    on_set_callback (int val)
					{
                        __on_set_callback_f_call_counter++;              // __
						D_int::set_value(val);
					}
	int             on_get_callback ()
					{
                        __on_get_callback_f_call_counter++;              // __
						return D_int::value();
					}
};






#endif /* __doocs_process_variable_adapter__ */

