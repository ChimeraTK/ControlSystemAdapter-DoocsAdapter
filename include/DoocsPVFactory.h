#ifndef CHIMERATK_DOOCS_PV_FACTORY_H
#define CHIMERATK_DOOCS_PV_FACTORY_H

#include <eq_fct.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariable.h>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/TypeChangingDecorator.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "VariableMapper.h"
#include "DoocsUpdater.h"
#include "PropertyDescription.h"

namespace ChimeraTK {

  /**
   * The DoocsProcessVariableFactory creates Doocs variables (D_fct) from ChimeraTK::ProcessVariables.
   */
  class DoocsPVFactory: boost::noncopyable {
  public:
    /**
     * The constructor needs a pointer to the EqFct. It is designed to be used inside the EqFct, so
     * the 'this' pointer will be given. As it is not copyable this is ok.
     */
    DoocsPVFactory(EqFct * const eqFct, DoocsUpdater & updater, boost::shared_ptr<ControlSystemPVManager> const & csPVManager);

    boost::shared_ptr<D_fct> create( std::shared_ptr<PropertyDescription> const & propertyDescription );
      
  protected:
    EqFct * _eqFct; //< The EqFct which is holding the factory. Needed in the constructor of the doocs properties.
    DoocsUpdater & _updater;
    boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager; //< The pv manager, needed to get the instances

    // create the DOOCS property. Note: DOOCS_T is only used for scalar properties, not for arrays!
    template<class T, class DOOCS_T>
    typename boost::shared_ptr<D_fct> createDoocsScalar(AutoPropertyDescription const & propertyDescription, DecoratorType decoratorType);
    /// @todo FIXME: use SpectrumDescription here
    boost::shared_ptr<D_fct> createDoocsSpectrum(SpectrumDescription const & spectrumDescription);

    boost::shared_ptr<D_fct> createDoocsArray(std::shared_ptr<ArrayDescription> const & spectrumDescription);

    template<class DOOCS_PRIMITIVE_T, class DOOCS_T>
    boost::shared_ptr<D_fct> typedCreateDoocsArray( ArrayDescription const & arrayDescription);


    boost::shared_ptr<D_fct> autoCreate( std::shared_ptr<PropertyDescription> const & propertyDescription);
 
  };


  // specialisation for strings
  template<>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsScalar<std::string, D_string>(AutoPropertyDescription const & propertyDescription, DecoratorType decoratorType);
  
  
}//namespace ChimeraTK

#endif // CHIMERATK_DOOCS_PV_FACTORY_H
