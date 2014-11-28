#ifndef __m4uD_xxx__
#define __m4uD_xxx__


#include <boost/bind.hpp>
#include <boost/function.hpp>




template <typename T>
class m4uD_xxx
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

};






#endif /* __m4uD_xxx__ */

