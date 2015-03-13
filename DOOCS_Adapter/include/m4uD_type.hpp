#ifndef __m4uD_type__
#define __m4uD_type__


#include <string>
#include <boost/bind.hpp>
#include <boost/function.hpp>




namespace mtca4u {




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


            // accessors with callbacks (not based on standard DOOCS property interface - a custom extension), for BL-side requests
	void	setval (T val)
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
	T	    getval ()
            {
                if (_onGetCallbackFunction)
                {
                    DOOCS_T::set_value( _onGetCallbackFunction () );
                }
                return DOOCS_T::value();
            }


            // accessors without callbacks (not based on standard DOOCS property interface - a custom extension), for BL-side requests
	void	setval_without_callback (T val)
            {
                DOOCS_T::set_value(val);
            }
	T   	getval_without_callback ()
            {
                return DOOCS_T::value();
            }


            // DOOCS standard interface override for CS-side requests
    void    get (EqAdr * _1, EqData * _2, EqData * _3, EqFct * _4)
            {
                if (_onGetCallbackFunction)
                {
                    DOOCS_T::set_value( _onGetCallbackFunction () );
                }
                DOOCS_T::get (_1, _2, _3, _4);
            }
    void    set (EqAdr * _1, EqData * _2, EqData * _3, EqFct * _4)
            {
                // cache the current value in the DOOCS property
                T oldVal = DOOCS_T::value();
                // set the new value of the variable
                DOOCS_T::set (_1, _2, _3, _4);
                // get the new value from the DOOCS property
                T val = DOOCS_T::value();
                if (_onSetCallbackFunction)
                {
                    // trigger the callback with both the new and the old value
                    _onSetCallbackFunction (val, oldVal);
                }
            }

};







template <>
class m4uD_type <std::string, D_string> : public D_string
{
protected:
	boost::function < void (std::string const &, std::string const & ) >	_onSetCallbackFunction;	// (newValue, oldValue)
	boost::function < std::string () >				            			_onGetCallbackFunction;
    
    
public:
    


    void setOnSetCallbackFunction (boost::function < void (std::string const &, std::string const & ) > onSetCallbackFunction)
         {
             _onSetCallbackFunction = onSetCallbackFunction;
         }


    void setOnGetCallbackFunction (boost::function < std::string () > onGetCallbackFunction)
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




    m4uD_type <std::string, D_string> (const char *pn, EqFct *ef) :    D_string(pn, ef)
            {
                D_string::set_value("<empty>");
            }


            // !!!: the getters/setters remain const char* -based, so as to retain compliance with the D_string interface   (FIXME: probably not needed anymore)
            // accessors with callbacks (not based on standard DOOCS property interface - a custom extension), for BL-side requests
	void	setval (const char * val)
            {
                // cache the current value in the DOOCS property
                std::string oldVal = std::string(D_string::value());
                // set the new value of the variable
                D_string::set_value(val);
                if (_onSetCallbackFunction)
                {
                    // trigger the callback with both the new and the old value
                    _onSetCallbackFunction (std::string(val), oldVal);
                }
            }
	const char *    getval ()
            {
                if (_onGetCallbackFunction)
                {
                    D_string::set_value( _onGetCallbackFunction().c_str() );
                }
                return D_string::value();
            }


            // accessors without callbacks (not based on standard DOOCS property interface - a custom extension), for BL-side requests
	void	setval_without_callback (const char * val)
            {
                D_string::set_value(val);
            }
	const char * 	getval_without_callback ()
            {
                return D_string::value();
            }


            // DOOCS standard interface override for CS-side requests
    void    get (EqAdr * _1, EqData * _2, EqData * _3, EqFct * _4)
            {
                if (_onGetCallbackFunction)
                {
                    D_string::set_value( _onGetCallbackFunction().c_str() );
                }
                D_string::get (_1, _2, _3, _4);
            }
    void    set (EqAdr * _1, EqData * _2, EqData * _3, EqFct * _4)
            {
                // cache the current value in the DOOCS property
                std::string oldVal = std::string(D_string::value());
                // set the new value of the variable
                D_string::set (_1, _2, _3, _4);
                // get the new value from the DOOCS property
                std::string val = std::string(D_string::value());
                if (_onSetCallbackFunction)
                {
                    // trigger the callback with both the new and the old value
                    _onSetCallbackFunction (val, oldVal);
                }
            }
};



} // namespace mtca4u


#endif /* __m4uD_type__ */

