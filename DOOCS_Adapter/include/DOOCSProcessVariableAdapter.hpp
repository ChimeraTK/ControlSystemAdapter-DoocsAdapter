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






#endif /* __doocs_process_variable_adapter__ */

