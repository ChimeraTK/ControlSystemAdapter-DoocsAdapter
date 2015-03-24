#ifndef __D_SettableSpectrum__
#define __D_SettableSpectrum__




namespace mtca4u {


template <typename T>
class D_SettableSpectrum : public D_spectrum
{
public:
 
    D_SettableSpectrum (const char *pn, u_int maxl, EqFct *ef) : D_spectrum  (pn, maxl, ef) {}



    void                 fill_spectrum (int index, T item) // "original" DOOCS
                         {
                             D_spectrum::fill_spectrum ( index, static_cast<float>(item) );
                         }
 
 
    std::vector<T> &     get_spectrum_copy (std::vector<T> & data) // a copier
                         {
                             assert ( data.size() == static_cast<size_t>(max_length_) );
                             float * buffer = spectrum()->d_spect_array.d_spect_array_val;
                             std::copy ( buffer, buffer+max_length_, data.begin() );
                             return data;
                         }
    void                 set_spectrum (const std::vector<T> & data)
                         {
                             assert ( data.size() == static_cast<size_t>(max_length_) );
                             float * buffer = spectrum()->d_spect_array.d_spect_array_val;
                             std::copy ( data.begin(), data.end(), buffer );
                         }
    void                 fill(T const & t)
                         {
                             float * buffer = spectrum()->d_spect_array.d_spect_array_val;
                             std::fill ( buffer, buffer+max_length_, t );
                         }
};



} //namespace mtca4u


#endif /* __D_SettableSpectrum__ */
