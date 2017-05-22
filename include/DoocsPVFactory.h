#ifndef CHIMERATK_DOOCS_PV_FACTORY_H
#define CHIMERATK_DOOCS_PV_FACTORY_H

#include <eq_fct.h>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariable.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

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
    DoocsPVFactory(EqFct * const eqFct, boost::shared_ptr<ControlSystemSynchronizationUtility> const & syncUtility);

    boost::shared_ptr<D_fct> create( ProcessVariable::SharedPtr & processVariable );

  protected:
    EqFct * _eqFct; //< The EqFct which is holding the factory. Needed in the constructor of the doocs properties.
    boost::shared_ptr<ControlSystemSynchronizationUtility> _syncUtility; //< The syncUtility is needed to register listeners

    // create the DOOCS property. Note: DOOCS_T and DOOCS_VALUE_T are only used for scalar properties, not for arrays!
    template<class T, class DOOCS_T, class DOOCS_VALUE_T>
    typename boost::shared_ptr<D_fct> createDoocsProperty(typename ProcessVariable::SharedPtr & processVariable);
 
  };


  // specialisation for strings
  template<>
  typename boost::shared_ptr<D_fct> DoocsPVFactory::createDoocsProperty<std::string, D_string, std::string>(typename ProcessVariable::SharedPtr & processVariable);
  
  
}//namespace ChimeraTK

#endif // CHIMERATK_DOOCS_PV_FACTORY_H
