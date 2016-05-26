#ifndef __dpvaf__
#define __dpvaf__

#include <boost/any.hpp>
#include <ControlSystemAdapter/ProcessVariableFactory.h>


#include <eq_fct.h>
#include "m4uD_type.hpp"
#include "DOOCSProcessVariableAdapter.hpp"

#include "m4uD_array.hpp"
#include "DOOCSProcessVariableArray.hpp"



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
        if(variableType == typeid(std::string) ) {

            boost::shared_ptr< m4uD_type<std::string, D_string> >   mydtype ( new m4uD_type<std::string, D_string> ( name.c_str(), ef ) );
            return boost::shared_ptr < ProcessVariable<std::string>  > ( new DOOCSProcessVariableAdapter<std::string , m4uD_type<std::string , D_string > > (mydtype) ); 

        } else
        {
          throw std::bad_typeid();
        }

    }
    

    boost::any createProcessArray ( std::type_info const & variableType, std::string name, size_t arraySize)
    {
        if(variableType == typeid(int) ) {

            boost::shared_ptr< mtca4u::m4uD_array<int> > darray ( new mtca4u::m4uD_array<int> ( name.c_str(), arraySize, ef ) );
            return boost::shared_ptr < ProcessArray<int> >      ( new mtca4u::DOOCSProcessVariableArray<int, mtca4u::m4uD_array<int> > (darray , arraySize));

        } else
        if(variableType == typeid(float) ) {

            boost::shared_ptr< mtca4u::m4uD_array<float> > darray ( new mtca4u::m4uD_array<float> ( name.c_str(), arraySize, ef ) );
            return boost::shared_ptr < ProcessArray<float> >      ( new mtca4u::DOOCSProcessVariableArray<float, mtca4u::m4uD_array<float> > (darray , arraySize));

        } else
        if(variableType == typeid(double) ) {

            boost::shared_ptr< mtca4u::m4uD_array<double> > darray ( new mtca4u::m4uD_array<double> ( name.c_str(), arraySize, ef ) );
            return boost::shared_ptr < ProcessArray<double> >      ( new mtca4u::DOOCSProcessVariableArray<double, mtca4u::m4uD_array<double> > (darray , arraySize));

        } else
        {
          throw std::bad_typeid();
        }
    }


public:

    DOOCSProcessVariableFactory (EqFct *_ef) : ef(_ef) {}
    
};


}// namespace mtca4u


#endif // __dpvaf__
