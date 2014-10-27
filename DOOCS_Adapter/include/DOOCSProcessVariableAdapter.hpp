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
	//~ D_int() : value_(0) {}
    
    D_int (const char *pn, EqFct *ef) : value_(0) {}
    D_int (const char *pn)            : value_(0) {}


	virtual void	set_value (int val)
					{
						value_ = val;
					}

	virtual int		value ()
					{
						return value_;
					}
};





class myD_int : public D_int
{
    
protected:
    DOOCSPVAdapter * doocs_adapter;     // DOOCS process variable adapter handler
	
public:
    
    myD_int (const char *pn, EqFct *ef, DOOCSPVAdapter * _da) : doocs_adapter(_da), D_int(pn, ef)
    {
        doocs_adapter->setOnSetCallbackFunction(boost::bind (&D_int::set_value, this, _1));     // original set_value() !!!
    }

	virtual void	set_value (int val)
					{
						D_int::set_value(val);
                        doocs_adapter->setWithoutCallback(val);
					}

};






#endif /* __doocs_process_variable_adapter__ */

