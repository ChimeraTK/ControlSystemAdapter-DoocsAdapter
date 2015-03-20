#ifndef eq_data_h
#define eq_data_h

#include  "eq_types.h"




class EqData {

private:
    SPECTRUM spectrum;

public:
    EqData (u_int _len) : spectrum (_len) {}
    
    SPECTRUM * get_spectrum (void)
    { 
            return &spectrum;
    }
};


        
#endif
