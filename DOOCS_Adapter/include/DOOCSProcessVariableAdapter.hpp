#ifndef __dpva__
#define __dpva__

#include <boost/function.hpp>

#include "ProcessVariable.h"


class DOOCSPVAdapter : public ProcessVariable < int >
{
protected:
    
    myD_int * mydint;
    
public:
    
    DOOCSPVAdapter (myD_int * _mydint) : mydint(_mydint) {};
    

    void setOnSetCallbackFunction( boost::function< void (int const & /*newValue*/, int const & /*oldValue*/) > onSetCallbackFunction)
         {
             mydint->setOnSetCallbackFunction(onSetCallbackFunction);
         }
    void setOnGetCallbackFunction( boost::function< int () > onGetCallbackFunction )
         {
             mydint->setOnGetCallbackFunction(onGetCallbackFunction);
         }
    
    void clearOnSetCallbackFunction() { mydint->clearOnSetCallbackFunction(); }
    void clearOnGetCallbackFunction() { mydint->clearOnGetCallbackFunction(); }
    

    void set(int const & t)
         {
             mydint->set_value(t);
         }
    
    int  get()
         {
             return mydint->value();
         }
    

    void setWithoutCallback(int const & t)
         {
             mydint->set_value_without_callback(t);
         }
    int  getWithoutCallback() const
         {
             return mydint->value_without_callback();
         }
    

    void set(ProcessVariable<int> const & other)
         {
             set( other.getWithoutCallback() );
         }
    void setWithoutCallback(ProcessVariable<int> const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
         }

    
    DOOCSPVAdapter & operator=(ProcessVariable<int> const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
         }
    DOOCSPVAdapter & operator=(DOOCSPVAdapter const & other)
         {
             setWithoutCallback( other.getWithoutCallback() );
         }
    DOOCSPVAdapter & operator=(int const & t)
         {
             setWithoutCallback( t );
         }

    // --- the rest (incl. operators) will follow ---
    
    //~ operator int () const {};
    


};


#endif /* __dpva__ */

