#ifndef __m4uD_type__
#define __m4uD_type__


#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "eq_fct.h"



template <typename T, typename DOOCS_T>
class m4uD_type : public DOOCS_T
{
protected:
	boost::function < void (T const &, T const & ) >	_onSetCallbackFunction;	// (newValue, oldValue)
	boost::function < T () >							_onGetCallbackFunction;
    
    
public:
    


    void setOnSetCallbackFunction (boost::function < void (T const &, T const & ) > onSetCallbackFunction)
         {
             _onSetCallbackFunction = onSetCallbackFunction;
         }


    void setOnGetCallbackFunction (boost::function < T () > onGetCallbackFunction)
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




    m4uD_type (const char *pn, EqFct *ef) :    DOOCS_T(pn, ef) {}


            // accessors with callbacks (based on standard DOOCS property interface)
	void	set_value (T val)
            {
                // cache the current value in the DOOCS property
                T oldVal = DOOCS_T::value();
                // set the new value of the variable
                DOOCS_T::set_value(val);
                if (_onSetCallbackFunction)
                {
                    // trigger the callback with both the new and the old value
                    _onSetCallbackFunction (val, oldVal);
                }
            }
	T	    value ()
            {
                if (_onGetCallbackFunction)
                {
                    DOOCS_T::set_value( _onGetCallbackFunction () );
                }
                return DOOCS_T::value();
            }


            // accessors without callbacks (extension of the original DOOCS property interface)
	void	set_value_without_callback (T val)
            {
                DOOCS_T::set_value(val);
            }
	T   	value_without_callback ()
            {
                return DOOCS_T::value();
            }

};






#endif /* __m4uD_type__ */

