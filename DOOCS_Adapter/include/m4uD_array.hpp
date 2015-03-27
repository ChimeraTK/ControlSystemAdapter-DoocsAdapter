#ifndef __m4uD_array__
#define __m4uD_array__


#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <ControlSystemAdapter/ProcessArray.h>

#include "D_CachedSpectrum.hpp"

namespace mtca4u {




template <typename T>
class m4uD_array : public D_CachedSpectrum<T>
{

protected:

    boost::function< void (ProcessArray<T> const & ) > _onSetCallbackFunction;    // newValue
    boost::function< void (ProcessArray<T> & ) >       _onGetCallbackFunction;    // toBeFilled

    //~ size_t             _size;            // FIXME: what was this one for?

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




    m4uD_array (const char *pn, int maxl, EqFct *ef) :  D_CachedSpectrum<T> (pn, maxl, ef)//,
                                                        //~ _size               (maxl)
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



    bool limitedPrecision() // a "default" (for T) is true, to be on a safe side
         {
             return true;
         }

};



///////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                                                     //
template<>
bool m4uD_array<float>::limitedPrecision()
{
    return false;
}



static const int32_t MIN_INT_IN_FLOAT = -16777216;
static const int32_t MAX_INT_IN_FLOAT =  16777215;


template<>
bool m4uD_array<int>::limitedPrecision()
{
    size_t _size = static_cast<size_t> (length());
    for( size_t i = 0; i < _size; ++i)
    {
        // <=, because -16777217 is rounded to -16777216, so there might be a rounding error.
        // For positive values 16777217 is rounded to 16777216, so 16777215 is safe.
        if ( (static_cast<int32_t>(read_spectrum(i)) <= MIN_INT_IN_FLOAT ) ||
             (static_cast<int32_t>(read_spectrum(i)) >  MAX_INT_IN_FLOAT )
           )
        {
            return true;
        }
    }
    return false;
}
                                                                                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////////



} // namespace mtca4u


#endif /* __m4uD_array__ */

