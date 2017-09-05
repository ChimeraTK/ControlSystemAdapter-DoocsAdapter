#ifndef CHIMERATK_DOOCS_PV_FACTORY_H
#define CHIMERATK_DOOCS_PV_FACTORY_H

#include <eq_fct.h>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariable.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "VariableMapper.h"

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
    DoocsPVFactory(EqFct * const eqFct, boost::shared_ptr<ControlSystemSynchronizationUtility> const & syncUtility, boost::shared_ptr<ControlSystemPVManager> const & csPVManager);

    boost::shared_ptr<D_fct> create( ProcessVariable::SharedPtr & processVariable );

    /// @todo FIXME: This is for brainstorming and quick protoryping. Did not want to call it create.
    boost::shared_ptr<D_fct> new_create( std::shared_ptr<PropertyDescription> const & propertyDescription );
      
  protected:
    EqFct * _eqFct; //< The EqFct which is holding the factory. Needed in the constructor of the doocs properties.
    boost::shared_ptr<ControlSystemSynchronizationUtility> _syncUtility; //< The syncUtility is needed to register listeners
    boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager; //< The pv manager, needed to get the instances @todo Do we need it in a proper design?

    // create the DOOCS property. Note: DOOCS_T is only used for scalar properties, not for arrays!
    template<class T, class DOOCS_T>
    typename boost::shared_ptr<D_fct> createDoocsProperty(typename ProcessVariable::SharedPtr & processVariable);
 
  };


  // specialisation for strings
  template<>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsProperty<std::string, D_string>(typename ProcessVariable::SharedPtr & processVariable);
  
  
}//namespace ChimeraTK

#endif // CHIMERATK_DOOCS_PV_FACTORY_H
