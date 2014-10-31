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
    
      
    int get()
    {
        return mydint->value();
    }
    
    // stubs
    void setWithoutCallback(int const & t) {};
    int  getWithoutCallback() const { return -1;}
    

    // --- the rest (incl. operators) will follow ---
    
    //~ void setWithoutCallback(ProcessVariable<int> const & other) {};
    //~ void set(ProcessVariable<int> const & other) {};
    //~ operator int () const {};
    //~ DOOCSPVAdapter & operator=(int const & t);
    //~ DOOCSPVAdapter & operator=(DOOCSPVAdapter const & other);


};


#endif /* __dpva__ */

