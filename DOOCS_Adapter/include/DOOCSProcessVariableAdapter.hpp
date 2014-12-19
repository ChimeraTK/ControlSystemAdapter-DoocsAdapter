#ifndef __dpva__
#define __dpva__

#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "ProcessVariable.h"


// friends
struct CallbacksTestFixture;
struct InterPVTestFixture;
class DOOCSProcessVariableFactory;

namespace mtca4u {




template <typename T, typename M4U_DOOCS_T>
class DOOCSProcessVariableAdapter : public ProcessVariable < T >
{
protected:
    
    boost::shared_ptr< M4U_DOOCS_T >   m4uD_type_T;
    
    // a non-public constructor
    DOOCSProcessVariableAdapter (boost::shared_ptr< M4U_DOOCS_T > _m4uD_type_T) : m4uD_type_T(_m4uD_type_T) {};
    
    // test structs to access the constructor
    friend struct ::CallbacksTestFixture;
    friend struct ::InterPVTestFixture;

    // the factory needs to be able to reach the constructor
    friend class DOOCSProcessVariableFactory;
    
public:
    

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

    
    DOOCSProcessVariableAdapter & operator=(ProcessVariable<T> const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
             return *this;
         }
    DOOCSProcessVariableAdapter & operator=(DOOCSProcessVariableAdapter const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
             return *this;
         }
    DOOCSProcessVariableAdapter & operator=(T const & t)
         {
             setWithoutCallback( t );
             return *this;
         }


    operator T () const
         {
             return m4uD_type_T->value_without_callback();
         }
};




template <>
class DOOCSProcessVariableAdapter <std::string, m4uD_type<std::string, D_string> > : public ProcessVariable < std::string >
{
protected:
    
    boost::shared_ptr< m4uD_type<std::string, D_string> >   m4uD_type_T;
    
    // a non-public constructor
    DOOCSProcessVariableAdapter <std::string, m4uD_type<std::string, D_string> > (boost::shared_ptr< m4uD_type<std::string, D_string> > _m4uD_type_T) : m4uD_type_T(_m4uD_type_T) {};

    // test structs to access the constructor
    friend struct ::CallbacksTestFixture;
    friend struct ::InterPVTestFixture;

    // the factory needs to be able to reach the constructor
    friend class DOOCSProcessVariableFactory;
    
public:
    

    void setOnSetCallbackFunction( boost::function< void (std::string const & /*newValue*/, std::string const & /*oldValue*/) > onSetCallbackFunction)
         {
             m4uD_type_T->setOnSetCallbackFunction(onSetCallbackFunction);
         }
    void setOnGetCallbackFunction( boost::function< std::string () > onGetCallbackFunction )
         {
             m4uD_type_T->setOnGetCallbackFunction(onGetCallbackFunction);
         }
    
    void clearOnSetCallbackFunction() { m4uD_type_T->clearOnSetCallbackFunction(); }
    void clearOnGetCallbackFunction() { m4uD_type_T->clearOnGetCallbackFunction(); }
    

    void set(std::string const & t)
         {
             m4uD_type_T->set_value(t.c_str());
         }
    
    std::string    get()
         {
             return std::string(m4uD_type_T->value());
         }
    

    void setWithoutCallback(std::string const & t)
         {
             m4uD_type_T->set_value_without_callback(t.c_str());
         }
    std::string    getWithoutCallback() const
         {
             return std::string(m4uD_type_T->value_without_callback());
         }
    

    void set(ProcessVariable<std::string> const & other)
         {
             set( other.getWithoutCallback() );
         }
    void setWithoutCallback(ProcessVariable<std::string> const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
         }

    
    DOOCSProcessVariableAdapter & operator=(ProcessVariable<std::string> const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
             return *this;
         }
    DOOCSProcessVariableAdapter & operator=(DOOCSProcessVariableAdapter const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
             return *this;
         }
    DOOCSProcessVariableAdapter & operator=(std::string const & t)
         {
             setWithoutCallback( t );
             return *this;
         }


    operator std::string () const
         {
             return std::string(m4uD_type_T->value_without_callback());
         }
};



}//namespace mtca4u


#endif /* __dpva__ */

