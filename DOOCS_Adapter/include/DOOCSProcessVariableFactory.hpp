#ifndef __dpvaf__
#define __dpvaf__

#include <boost/any.hpp>
#include "ProcessVariableFactory.h"


#include "eq_fct.h"
#include "m4uD_type.hpp"
#include "DOOCSProcessVariableAdapter.hpp"



namespace mtca4u{


class DOOCSProcessVariableFactory : public ProcessVariableFactory {

    EqFct *ef;
    
protected:
    
    boost::any createProcessVariable ( std::type_info const & variableType, std::string name ) 
    {
        
        
        if(variableType == typeid(int) ) {

            boost::shared_ptr< m4uD_type<int, D_int> >   mydtype ( new m4uD_type<int, D_int> ( name.c_str(), ef ) );
            return boost::shared_ptr < ProcessVariable<int>    > ( new DOOCSProcessVariableAdapter<int   , m4uD_type<int   , D_int   > > (mydtype) );

        } else
        if(variableType == typeid(float) ) {

            boost::shared_ptr< m4uD_type<float, D_float> >   mydtype ( new m4uD_type<float, D_float> ( name.c_str(), ef ) );
            return boost::shared_ptr < ProcessVariable<float>  > ( new DOOCSProcessVariableAdapter<float , m4uD_type<float , D_float > > (mydtype) ); 

        } else
        if(variableType == typeid(double) ) {

            boost::shared_ptr< m4uD_type<double, D_double> >   mydtype ( new m4uD_type<double, D_double> ( name.c_str(), ef ) );
            return boost::shared_ptr < ProcessVariable<double> > ( new DOOCSProcessVariableAdapter<double, m4uD_type<double, D_double> > (mydtype) );

        } else
        // TODO STRING!!!
        {
          throw std::bad_typeid();
        }

    }
    
    boost::any createProcessArray    ( std::type_info const & variableType, std::string name, size_t arraySize) { return NULL; } // TODO proper implementation to come after arrays support

public:

    DOOCSProcessVariableFactory (EqFct *_ef) : ef(_ef) {}
    
};


}// namespace mtca4u


#endif // __dpvaf__
