#ifndef __doocs_process_variable_adapter__
#define __doocs_process_variable_adapter__

#include <boost/function.hpp>



/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_int
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_int::set_value
 */
class D_int
{

	int				value_;
	
public:
	D_int() : value_(0) {}

	virtual void	set_value (int val)
					{
						value_ = val;
					}

	virtual int		value ()
					{
						return value_;
					}
};




class DOOCSPVAdapter
{
	
	int _t;
	
	boost::function < void (int const &, int const & ) >	_onSetCallbackFunction;	// (newValue, oldValue)
	boost::function < int () >								_onGetCallbackFunction;


public:

	DOOCSPVAdapter () {};

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

#endif /* __doocs_process_variable_adapter__ */

