#ifndef __m4uD_array__
#define __m4uD_array__


#include <boost/bind.hpp>
#include <boost/function.hpp>




namespace mtca4u {




template <typename T>
class m4uD_array : public D_spectrum
{

protected:

    boost::function< void (ProcessArray<T> const & ) > _onSetCallbackFunction;  // newValue
    boost::function< void (ProcessArray<T> & ) >       _onGetCallbackFunction;    // toBeFilled

    size_t             _size;
    std::vector<T>     _cache;
    
    bool                cache_synced;


    void sync_cache()
    {
        if ( !cache_synced )
            {
            for (int i=0; i<max_length_; ++i)
            {
                _cache[i] = static_cast<T> ( D_spectrum::read_spectrum(i) );            // FIXME: any chance for a more efficient way?
            }
            cache_synced = true;
        }
    }

public:


    void setOnSetCallbackFunction (boost::function< void (ProcessArray<T> const & ) > onSetCallbackFunction)
         {
             _onSetCallbackFunction = onSetCallbackFunction;
         }

    void setOnGetCallbackFunction (boost::function< void (ProcessArray<T> & ) > onGetCallbackFunction)
         {
             _onGetCallbackFunction = onGetCallbackFunction;
         }

    void clearOnSetCallbackFunction ()
         {
             _onSetCallbackFunction.clear ();
         }

    void clearOnGetCallbackFunction ()
         {
             _onGetCallbackFunction.clear ();
         }




    m4uD_array (const char *pn, int maxl, EqFct *ef) :  D_spectrum (pn, maxl, ef),
                                                        _size      (maxl)        ,
                                                        _cache     (maxl)
                                                    {
                                                        cache_synced = false;
                                                    }
    
    /** DEBUG */    bool __is_cache_synced_flag()
                         {
                             return cache_synced;
                         }

    /** DEBUG */    bool __is_cache_synced_vect()
                         {
                             for (int i=0; i<max_length_; ++i)
                             {
                                 if (_cache[i] != D_spectrum::read_spectrum(i))
                                     return false;
                             }
                             return true;
                         }



    // operation on single data elements is based on the following part of the D_spectrum interface:
    //  void  D_spectrum::fill_spectrum (int index, float item)
    //  float D_spectrum::read_spectrum (int index) const                       // FIXME : potrzebne sparametryzowanie (update: po co?)
                                                                                // FIXME2: callback calls to be moved into there (probably)


    /** overrides
     *  void   D_spectrum::fill_spectrum   ( int index, float item )
     */
    void  fill_spectrum (int index, float item)
          {
              D_spectrum::fill_spectrum (index, item);
              cache_synced = false;
          }


    // treat whole spectrum at once - for set and get, extending D_spectrum interface
    // // with callbacks
    void  fill_whole_spectrum (const std::vector<T> & data, ProcessArray<T> const & pa)
          {
              fill_whole_spectrum_without_callback(data);
              if (_onSetCallbackFunction){
                  _onSetCallbackFunction( pa );        // FIXME: here rather _onSetCallbackFunction(*this); or sth instead of the whole line
              }
          }
    const std::vector<T> & read_whole_spectrum ()        // FIXME: a copy return
          {
              if (_onGetCallbackFunction){
                  StubProcessArray<T> pa(5); _onGetCallbackFunction( pa );        // FIXME: here rather _onGetCallbackFunction(*this); or sth instead of the whole line, then test if "container" successfully filled
              }
              return read_whole_spectrum_without_callback();        // FIXME: make sure returned is what comes from callback
          }

    // // without callbacks
    void  fill_whole_spectrum_without_callback (const std::vector<T> & data)
          {
              for (size_t i=0; i<data.size(); ++i)
              {
                  D_spectrum::fill_spectrum (i, data[i]);               // FIXME: any chance for a more efficient way?
              }
              cache_synced = false;
          }
    const std::vector<T> & read_whole_spectrum_without_callback ()        // FIXME: a copy return (get rid of the std::vector<T> v below)
          {
              sync_cache();
              return _cache;
          }



    void  fillVector(std::vector <T> & toBeFilled)
          {
              sync_cache();
              std::copy(_cache.begin(), _cache.end(), toBeFilled.begin());
          }

};





}//namespace mtca4u


#endif /* __m4uD_array__ */

