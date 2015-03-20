#ifndef __SettableSpectrum__
#define __SettableSpectrum__


#include "d_fct.h"



class SettableSpectrum : public D_spectrum                                                                                                     //
{                                                                                                                                              //
public:                                                                                                                                        //
                                                                                                                                               //
    SettableSpectrum (const char *pn, u_int maxl, EqFct *ef)                          : D_spectrum  (pn, maxl, ef)                  {}         //
                                                                                                                                               //
                                                                                                                                               //
    inline std::vector<float> & get_spectrum (std::vector<float> & data)                                                                       //
                                {                                                                                                              //
                                    assert ( data.size() == static_cast<size_t>(max_length_) );                                                //
                                    float * buffer = spectrum()->d_spect_array.d_spect_array_val;                                              //
                                    std::copy ( buffer, buffer+max_length_, data.begin() );                                                    //
                                    return data;                                                                                               //
                                }                                                                                                              //
    inline void                 set_spectrum (const std::vector<float> & data)                                                                 //
                                {                                                                                                              //
                                    assert ( data.size() == static_cast<size_t>(max_length_) );                                                //
                                    float * buffer = spectrum()->d_spect_array.d_spect_array_val;                                              //
                                    std::copy ( data.begin(), data.end(), buffer );                                                            //
                                }                                                                                                              //
};                                                                                                                                             //





#endif /* __SettableSpectrum__ */
