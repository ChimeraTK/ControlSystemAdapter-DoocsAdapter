#ifndef __m4uD_array__
#define __m4uD_array__


#include <boost/bind.hpp>
#include <boost/function.hpp>




namespace mtca4u {




//~ template <typename T, typename DOOCS_T>
class m4uD_array : public D_spectrum
{

protected:
    boost::function< void (ProcessArray<float> const & ) > _onSetCallbackFunction;  // newValue
    boost::function< void (ProcessArray<float> & ) >       _onGetCallbackFunction;    // toBeFilled

    
public:


    void setOnSetCallbackFunction (boost::function< void (ProcessArray<float> const & ) > onSetCallbackFunction)
         {
             _onSetCallbackFunction = onSetCallbackFunction;
         }

    void setOnGetCallbackFunction (boost::function< void (ProcessArray<float> & ) > onGetCallbackFunction)
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




    // foundation for random access in the adapter is based on the following part of the D_spectrum interface:
    //  void  D_spectrum::fill_spectrum (int index, float item)
    //  float D_spectrum::read_spectrum (int index) const


    // treat whole spectrum at once - for set and get, extending D_spectrum interface
    // // with callbacks
    void  fill_whole_spectrum (const std::vector<float> & data)
          {
              fill_whole_spectrum_without_callback(data);
              if (_onSetCallbackFunction){
                  StubProcessArray<float> pa(5); _onSetCallbackFunction( pa );        // FIXME: here rather _onSetCallbackFunction(*this); or sth instead of the whole line
              }
          }
    std::vector<float> read_whole_spectrum () const        // FIXME: a copy return
          {
              if (_onGetCallbackFunction){
                  StubProcessArray<float> pa(5); _onGetCallbackFunction( pa );        // FIXME: here rather _onGetCallbackFunction(*this); or sth instead of the whole line
              }
              return read_whole_spectrum_without_callback();        // FIXME: make sure returned is what comes from callback
          }

    // // without callbacks
    void  fill_whole_spectrum_without_callback (const std::vector<float> & data)
          {
              for (size_t i=0; i<data.size(); ++i)  // size agreement checked by the adapter
              {
                  D_spectrum::fill_spectrum (i, data[i]);
              }
          }
    std::vector<float> read_whole_spectrum_without_callback () const        // FIXME: a copy return
          {
              std::vector<float> v;
              for (int i=0; i<max_length_; ++i)  // size agreement checked by the adapter
              {
                  v.push_back(D_spectrum::read_spectrum(i));
              }
              return v;
          }

};





}//namespace mtca4u


#endif /* __m4uD_array__ */

