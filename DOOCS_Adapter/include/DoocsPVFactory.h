#ifndef MTCA4U_DOOCS_PV_FACTORY_H
#define MTCA4U_DOOCS_PV_FACTORY_H

#include <eq_fct.h>
#include <ControlSystemAdapter/ControlSystemSynchronizationUtility.h>
#include <ControlSystemAdapter/ProcessVariable.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace mtca4u {

  /**
   * The DoocsProcessVariableFactory creates Doocs variables (D_fct) from mtca4u::ProcessVariables.
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

    template<class T, class DOOCS_T, class DOOCS_VALUE_T>
    typename  boost::shared_ptr<D_fct> createDoocsScalar(typename ProcessVariable::SharedPtr & processVariable);

    template<class T>
    typename  boost::shared_ptr<D_fct> createDoocsArray(typename ProcessVariable::SharedPtr & processVariable);
 
  };

}//namespace mtca4u

#endif // MTCA4U_DOOCS_PV_FACTORY_H
