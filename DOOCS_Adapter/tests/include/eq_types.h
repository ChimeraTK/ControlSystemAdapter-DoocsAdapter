#ifndef eq_types_h
#define eq_types_h



#include <algorithm>


typedef long     time_t;
typedef unsigned u_int;



struct Dspectarray {

    u_int     d_spect_array_len;
    float   * d_spect_array_val;

    Dspectarray (u_int _len) : d_spect_array_len(_len)
    {
        d_spect_array_val = new float[_len];
        std::fill (d_spect_array_val, d_spect_array_val+d_spect_array_len, 0);
    }

    ~Dspectarray ()
    {
        delete [] d_spect_array_val;
    }

};


struct SPECTRUM {
        struct {
            u_int comment_len;
            char  *comment_val;
        }      comment;
        time_t tm;
        float  s_start;
        float  s_inc;
        u_int  status;
        Dspectarray d_spect_array;

        SPECTRUM (u_int _len) : d_spect_array(_len) {}
};



#endif  /* eq_types_h */

