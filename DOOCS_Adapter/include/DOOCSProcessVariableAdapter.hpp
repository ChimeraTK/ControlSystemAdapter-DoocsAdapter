#ifndef __dpva__
#define __dpva__


#include <boost/function.hpp>

#include "ProcessVariable.h"


namespace mtca4u {



template <typename T, typename M4U_DOOCS_T>
class DOOCSPVAdapter : public ProcessVariable < T >
{
protected:
    
    M4U_DOOCS_T  * m4uD_type_T;
    
public:
    
    DOOCSPVAdapter (M4U_DOOCS_T * _m4uD_type_T) : m4uD_type_T(_m4uD_type_T) {};
    

    void setOnSetCallbackFunction( boost::function< void (T const & /*newValue*/, T const & /*oldValue*/) > onSetCallbackFunction)
         {
             m4uD_type_T->setOnSetCallbackFunction(onSetCallbackFunction);
         }
    void setOnGetCallbackFunction( boost::function< T () > onGetCallbackFunction )
         {
             m4uD_type_T->setOnGetCallbackFunction(onGetCallbackFunction);
         }
    
    void clearOnSetCallbackFunction() { m4uD_type_T->clearOnSetCallbackFunction(); }
    void clearOnGetCallbackFunction() { m4uD_type_T->clearOnGetCallbackFunction(); }
    

    void set(T const & t)
         {
             m4uD_type_T->set_value(t);
         }
    
    T    get()
         {
             return m4uD_type_T->value();
         }
    

    void setWithoutCallback(T const & t)
         {
             m4uD_type_T->set_value_without_callback(t);
         }
    T    getWithoutCallback() const
         {
             return m4uD_type_T->value_without_callback();
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
             return m4uD_type_T->value_without_callback();
         }
};


}//namespace mtca4u


#endif /* __dpva__ */

