#ifndef __dpvaarray__
#define __dpvaarray__


#include <stdexcept>

#include <algorithm>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "ProcessArray.h"

#include <execinfo.h>


// friends
//struct TestFixture;
//struct InterPVTestFixture;
//~ class DOOCSProcessVariableFactory;


namespace mtca4u {


template <typename T, typename M4U_DOOCSARR_T>
class DOOCSProcessVariableArray : public ProcessArray<T>
{


protected:
    boost::shared_ptr< M4U_DOOCSARR_T >   m4uD_array_T;


    size_t            _nValidElements;    ///< The information how many of the elements are valid.      // FIXME: relevant?
    
    boost::function< void (ProcessArray<T> const & ) > _onSetCallbackFunction;         // newValue
    boost::function< void (ProcessArray<T>       & ) > _onGetCallbackFunction;     // toBeFilled
    
    
    // test structs to access the constructor
//    friend struct ::TestFixture;
    //~ friend class DOOCSProcessVariableFactory;	// The factory is friend because someone has to be able to construct


public:

    DOOCSProcessVariableArray(boost::shared_ptr< M4U_DOOCSARR_T > _m4uD_array_T, size_t arraySize) : m4uD_array_T(_m4uD_array_T), _nValidElements(arraySize){}		// The constructors are private and can only be created by the factory.



    void    setOnSetCallbackFunction( boost::function< void (ProcessArray<T> const & ) > onSetCallbackFunction )
            {
                m4uD_array_T->setOnSetCallbackFunction(onSetCallbackFunction);
            }

    void    setOnGetCallbackFunction( boost::function< void (ProcessArray<T>       & ) > onGetCallbackFunction )
            {
                m4uD_array_T->setOnGetCallbackFunction(onGetCallbackFunction);
            }

    void    clearOnSetCallbackFunction() { m4uD_array_T->clearOnSetCallbackFunction(); }
    void    clearOnGetCallbackFunction() { m4uD_array_T->clearOnGetCallbackFunction(); }

	

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                                                                                    //
    DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > & operator=(DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > const & other)
            {
                if(&other==this){
                    return (*this);
                }
                setWithoutCallback(other);
                return (*this);
            }
        
    DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > & operator=(ProcessArray<T> const & other)
            {
                setWithoutCallback(other);
                return (*this);
            }
        
    DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > & operator=(std::vector<T> const & v)
            {
                setWithoutCallback(v);
                return *this;
            }
                                                                                                                                    //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    void    setWithoutCallback
            (ProcessArray<T> const & other)
            {
                if (other.size() != size()){
                    throw std::out_of_range("Assigned vector size mismatch.");
                }
                
                if ( typeid(other) == typeid(DOOCSProcessVariableArray< T, M4U_DOOCSARR_T >) )
                {
                    DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > const * otherDoocsProcessArray = static_cast< DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > const * >( & other );
                    m4uD_array_T->setspectrum_without_callback (otherDoocsProcessArray->m4uD_array_T->getspectrum_without_callback ());
                } else
                {
                    typename ProcessArray<T>::const_iterator it;
                    size_t idx;
                    for (it = other.cbegin(), idx=0; it !=other.cend(); ++it, ++idx) {
                        m4uD_array_T->fill_spectrum (static_cast<int>(idx), *it);   // cannot avoid "fill_spectrum" here, if "other" relies on iterators
                    }
                }
            }

    void    setWithoutCallback
            (std::vector<T> const & v)
            {
                if (v.size() != size()){
                    throw std::out_of_range("Assigned vector size mismatch.");
                }
                m4uD_array_T->setspectrum_without_callback (v);
            }
			  
    void    set
            (ProcessArray<T> const & other)
            {
                if (other.size() != size()){
                    throw std::out_of_range("Assigned vector size mismatch.");
                }
                
                if ( typeid(other) == typeid(DOOCSProcessVariableArray< T, M4U_DOOCSARR_T >) )
                {
                    DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > const * otherDoocsProcessArray = static_cast< DOOCSProcessVariableArray< T, M4U_DOOCSARR_T > const * >( & other );
                    m4uD_array_T->setspectrum (otherDoocsProcessArray->m4uD_array_T->getspectrum_without_callback(), *this);
                } else
                {
                    typename ProcessArray<T>::const_iterator it;
                    std::vector<T> otherValuesVector;
                    for (it = other.cbegin(); it !=other.cend(); ++it) {    // cannot avoid creating a vector out of "other" in order to feed with it the m4uD_array_T
                        otherValuesVector.push_back(*it);                   //   if "other" relies on iterators
                    }
                    m4uD_array_T->setspectrum (otherValuesVector, *this);
                }
            }

    void    set
            (std::vector<T> const & v)
            {
                if (v.size() != size()){
                    throw std::out_of_range("Assigned vector size mismatch.");
                }
                m4uD_array_T->setspectrum(v, *this);
            }


    std::vector<T> const & get()
            {
                return m4uD_array_T->getspectrum(*this);
            }

    std::vector<T> const & getWithoutCallback()
            {
                return m4uD_array_T->getspectrum_without_callback();
            }

    void    fillVector(std::vector <T> & toBeFilled)
            {
                if (toBeFilled.size() != size()){
                    throw std::out_of_range("Assigned vector size mismatch.");
                }
                m4uD_array_T->fillVector(toBeFilled);
            }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                                                               //
    size_t  size() const
            {
                return static_cast<size_t>(m4uD_array_T->length());
            }

    bool    empty() const
            {
                return size() == 0;
            }

    void    fill(T const & t)
            {
                m4uD_array_T->fill(t);
            }  
                                                                                                               //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////


    bool    limitedPrecision()
            {
                return m4uD_array_T->limitedPrecision();
            }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                                                               //
            T & operator[](size_t index){
                //~ return _container[index];
                (void)index; // suppress [-Wunused-parameter]
                throw std::logic_error("\"operator[]_1\" - The method or operation is not implemented.");
            }

            T const & operator[](size_t index) const{
                //~ return _container[index];    
                (void)index; // suppress [-Wunused-parameter]
                throw std::logic_error("\"operator[]_2\" - The method or operation is not implemented.");
            }

            T & at(size_t index){
                //~ return m4uD_array_T->read_spectrum(static_cast<int>index);
                (void)index; // suppress [-Wunused-parameter]
                throw std::logic_error("\"at_1\" - The method or operation is not implemented.");
            }

            T const & at(size_t index) const{
                //~ return _container.at(index);
                (void)index; // suppress [-Wunused-parameter]
                throw std::logic_error("\"at_2\" - The method or operation is not implemented.");
            }

            T & front(){
                //~ return _container.front();
                throw std::logic_error("\"front_1\" - The method or operation is not implemented.");
            }

            T const & front() const{
                //~ return _container.front();
                throw std::logic_error("\"front_2\" - The method or operation is not implemented.");
            }

            T & back(){
                //~ return _container.back();
                throw std::logic_error("\"back_1\" - The method or operation is not implemented.");
            }

            T const & back() const{
                //~ return _container.back();
                throw std::logic_error("\"back_2\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::iterator begin(){
                //~ return typename mtca4u::ProcessArray<T>::iterator(_container.begin());
                throw std::logic_error("\"iterator_1\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::iterator end(){
                //~ return typename mtca4u::ProcessArray<T>::iterator(_container.end());
                throw std::logic_error("\"iterator_2\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_iterator begin() const{
                //~ return typename mtca4u::ProcessArray<T>::const_iterator(_container.begin());
                throw std::logic_error("\"iterator_3\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_iterator end() const{
                //~ return typename mtca4u::ProcessArray<T>::const_iterator(_container.end());
                throw std::logic_error("\"iterator_4\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_iterator cbegin() const{
                //~ return typename mtca4u::ProcessArray<T>::const_iterator(_container.begin());
                throw std::logic_error("\"iterator_5\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_iterator cend() const{
                //~ return typename mtca4u::ProcessArray<T>::const_iterator(_container.end());
                throw std::logic_error("\"iterator_6\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::reverse_iterator rbegin(){
                //~ return typename mtca4u::ProcessArray<T>::reverse_iterator(_container.rbegin());
                throw std::logic_error("\"iterator_7\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::reverse_iterator rend(){
                //~ return typename mtca4u::ProcessArray<T>::reverse_iterator(_container.rend());
                throw std::logic_error("\"iterator_8\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_reverse_iterator rbegin() const{
                //~ return typename mtca4u::ProcessArray<T>::const_reverse_iterator(_container.rbegin());
                throw std::logic_error("\"iterator_9\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_reverse_iterator rend() const{
                //~ return typename mtca4u::ProcessArray<T>::const_reverse_iterator(_container.rend());
                throw std::logic_error("\"iterator_10\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_reverse_iterator crbegin() const{
                //~ return typename mtca4u::ProcessArray<T>::const_reverse_iterator(_container.rbegin());
                throw std::logic_error("\"iterator_11\" - The method or operation is not implemented.");
            }

            typename mtca4u::ProcessArray<T>::const_reverse_iterator crend() const{
                //~ return typename mtca4u::ProcessArray<T>::const_reverse_iterator(_container.rend());
                throw std::logic_error("\"iterator_12\" - The method or operation is not implemented.");
            }
                                                                                                               //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
};


} //namespace mtca4u

#endif /* __dpvaarray__ */
