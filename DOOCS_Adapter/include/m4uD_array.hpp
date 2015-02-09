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




    m4uD_array (const char *pn, int maxl, EqFct *ef) :    D_spectrum(pn, maxl, ef) {}




    // operation on single data elements is based on the following part of the D_spectrum interface:
    //  void  D_spectrum::fill_spectrum (int index, float item)
    //  float D_spectrum::read_spectrum (int index) const                       // FIXME : potrzebne sparametryzowanie
                                                                                // FIXME2: callback calls to be moved into there (probably)

    // treat whole spectrum at once - for set and get, extending D_spectrum interface
    // // with callbacks
    void  fill_whole_spectrum (const std::vector<T> & data, ProcessArray<T> const & pa)
          {
              fill_whole_spectrum_without_callback(data);
              if (_onSetCallbackFunction){
                  _onSetCallbackFunction( pa );        // FIXME: here rather _onSetCallbackFunction(*this); or sth instead of the whole line
              }
          }
    std::vector<T> read_whole_spectrum () const        // FIXME: a copy return
          {
              if (_onGetCallbackFunction){
                  StubProcessArray<T> pa(5); _onGetCallbackFunction( pa );        // FIXME: here rather _onGetCallbackFunction(*this); or sth instead of the whole line
              }
              return read_whole_spectrum_without_callback();        // FIXME: make sure returned is what comes from callback
          }

    // // without callbacks
    void  fill_whole_spectrum_without_callback (const std::vector<T> & data)
          {
              for (size_t i=0; i<data.size(); ++i)  // size agreement checked by the adapter
              {
                  D_spectrum::fill_spectrum (i, data[i]);               // FIXME: any chance for a more efficient way?
              }
          }
    std::vector<T> read_whole_spectrum_without_callback () const        // FIXME: a copy return
          {
              std::vector<T> v;
              for (int i=0; i<max_length_; ++i)  // size agreement checked by the adapter
              {
                  v.push_back(D_spectrum::read_spectrum(i));            // FIXME: any chance for a more efficient way?
              }
              return v;
          }

};





}//namespace mtca4u


#endif /* __m4uD_array__ */

