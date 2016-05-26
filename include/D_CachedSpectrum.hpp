#ifndef __D_CachedSpectrum__
#define __D_CachedSpectrum__


#include "D_SettableSpectrum.hpp"



namespace mtca4u {


template <typename T>
class D_CachedSpectrum : public D_SettableSpectrum<T>
{

protected:

    std::vector<T>     _cache;
    bool                cache_synced;


    void sync_cache()
    {
        if ( !cache_synced )
        {
            D_SettableSpectrum<T>::get_spectrum_copy(_cache);
            cache_synced = true;
        }
    }



public:

    D_CachedSpectrum (const char *pn, u_int maxl, EqFct *ef) : D_SettableSpectrum<T>  (pn, maxl, ef),
                                                               _cache                 (maxl, 0)
                                                               {
                                                                   cache_synced = false;
                                                               }

    /** DEBUG */    bool __is_cache_synced_flag()           // FIXME -> protected (?)
                         {
                             return cache_synced;
                         }

    /** DEBUG */    bool __is_cache_synced_vect()
                         {
                             for (int i=0; i<D_spectrum::max_length_; ++i)
                             {
                                 if (_cache[i] != D_spectrum::read_spectrum(i))  // DEBUG, so can stay like that
                                     return false;
                             }
                             return true;
                         }


    // "get" group
    void                   fillVector(std::vector <T> & toBeFilled)   // a copier
                           {
                               sync_cache();
                               std::copy(_cache.begin(), _cache.end(), toBeFilled.begin());
                           }
    std::vector<T> const & get_spectrum ()       // a non-copier
                           {
                               sync_cache();
                               return _cache;
                           }

    // "set" group
    void                   fill_spectrum (int index, T item)
                           {
                               cache_synced = false;
                               D_SettableSpectrum<T>::fill_spectrum (index, item);
                           }
    void                   fill (T const & t)
                           {
                               cache_synced = false;
                               D_SettableSpectrum<T>::fill (t);
                           }
    void                   set_spectrum (const std::vector<T> & data)
                           {
                               cache_synced = false;
                               D_SettableSpectrum<T>::set_spectrum (data);
                           }

};


} //namespace mtca4u



#endif /* __D_CachedSpectrum__ */
