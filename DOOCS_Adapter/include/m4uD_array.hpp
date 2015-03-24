#ifndef __m4uD_array__
#define __m4uD_array__


#include <boost/bind.hpp>
#include <boost/function.hpp>



#include "D_CachedSpectrum.hpp"

namespace mtca4u {




template <typename T>
class m4uD_array : public D_CachedSpectrum<T>
{

protected:

    boost::function< void (ProcessArray<T> const & ) > _onSetCallbackFunction;    // newValue
    boost::function< void (ProcessArray<T> & ) >       _onGetCallbackFunction;    // toBeFilled

    size_t             _size;            // FIXME: what was this one for?

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




    m4uD_array (const char *pn, int maxl, EqFct *ef) :  D_CachedSpectrum<T> (pn, maxl, ef),
                                                        _size               (maxl)
                                                     {}
    

    // treat whole spectrum at once - for set and get
    // // with callbacks
    void  setspectrum (const std::vector<T> & data, ProcessArray<T> const & pa)
          {
              setspectrum_without_callback(data);
              if (_onSetCallbackFunction){
                  _onSetCallbackFunction( pa );
              }
          }
    const std::vector<T> & getspectrum (ProcessArray<T> & pa)
          {
              if (_onGetCallbackFunction){
                  _onGetCallbackFunction( pa ); 
              }
              return getspectrum_without_callback();        // FIXME: make sure returned is what comes from callback
          }

    // // without callbacks
    void  setspectrum_without_callback (const std::vector<T> & data)
          {
              D_CachedSpectrum<T>::set_spectrum(data);
          }
    const std::vector<T> & getspectrum_without_callback ()
          {
              return D_CachedSpectrum<T>::get_spectrum();
          }

};





}//namespace mtca4u


#endif /* __m4uD_array__ */

