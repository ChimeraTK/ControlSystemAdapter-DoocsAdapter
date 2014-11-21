#ifndef __dpva__
#define __dpva__


#include <boost/function.hpp>

#include "ProcessVariable.h"


namespace mtca4u {



template <typename T, typename myD_T>
class DOOCSPVAdapter : public ProcessVariable < T >
{
protected:
    
    myD_T  * mydT;
    
public:
    
    DOOCSPVAdapter (myD_T * _mydT) : mydT(_mydT) {};
    

    void setOnSetCallbackFunction( boost::function< void (T const & /*newValue*/, T const & /*oldValue*/) > onSetCallbackFunction)
         {
             mydT->setOnSetCallbackFunction(onSetCallbackFunction);
         }
    void setOnGetCallbackFunction( boost::function< T () > onGetCallbackFunction )
         {
             mydT->setOnGetCallbackFunction(onGetCallbackFunction);
         }
    
    void clearOnSetCallbackFunction() { mydT->clearOnSetCallbackFunction(); }
    void clearOnGetCallbackFunction() { mydT->clearOnGetCallbackFunction(); }
    

    void set(T const & t)
         {
             mydT->set_value(t);
         }
    
    T    get()
         {
             return mydT->value();
         }
    

    void setWithoutCallback(T const & t)
         {
             mydT->set_value_without_callback(t);
         }
    T    getWithoutCallback() const
         {
             return mydT->value_without_callback();
         }
    

    void set(ProcessVariable<T> const & other)
         {
             set( other.getWithoutCallback() );
         }
    void setWithoutCallback(ProcessVariable<T> const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
         }

    
    DOOCSPVAdapter & operator=(ProcessVariable<T> const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
             return *this;
         }
    DOOCSPVAdapter & operator=(DOOCSPVAdapter const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
             return *this;
         }
    DOOCSPVAdapter & operator=(T const & t)
         {
             setWithoutCallback( t );
             return *this;
         }


    operator T () const
         {
             return mydT->value_without_callback();
         }
};


}//namespace mtca4u


#endif /* __dpva__ */

