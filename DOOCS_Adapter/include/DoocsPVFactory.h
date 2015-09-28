#ifndef MTCA4U_DOOCS_PV_FACTORY_H
#define MTCA4U_DOOCS_PV_FACTORY_H

#include <eq_fct.h>
#include <ControlSystemAdapter/ControlSystemPVFactory.h>
#include <boost/noncopyable.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>


namespace mtca4u {

  /**
   * The control-system PV factory for DoocsProcessVariables.
   */
  class DoocsPVFactory: public ControlSystemPVFactory, boost::noncopyable {
  public:
    /**
     * The constructor need a pointer to the EqFct. It is designed to be used inside the EqFct, so
     * the 'this' pointer will be given. As it is not copyable this is ok.
     */
    DoocsPVFactory(EqFct *eqFct);

    ControlSystemProcessVariable::SharedPtr createProcessScalar(
        const std::string& processVariableName,
        const std::type_info& valueType);

    ControlSystemProcessVariable::SharedPtr createProcessArray(
        const std::string& processVariableName, size_t size,
        const std::type_info& elementType, bool swappable);

    /** Doocs variables need to tell the PV-Manager that they need to be synchronised to the device side.
     *  As the factory is needed when constructing the managers, it has to be set later.
     */
    void setDoocsPVManager( boost::shared_ptr<DoocsPVManager> doocsPVManager );

  private:
    EqFct * _eqFct; //< The EqFct which is holding the factory. Needed in the constructor of the doocs properties.
    boost::weak_ptr<DoocsPVManager> _pvManager; //< The doocs process variables have to notify the pvManager that they ahve been changed.

  //  template<class T>
  //    typename ControlSystemProcessArray<T>::SharedPtr createProcessArrayInternal(
  //     const std::string& name, size_t size);

  template<class T, class DOOCS_T>
    typename ControlSystemProcessScalar<T>::SharedPtr createProcessScalarInternal( const std::string& name);
 
  };

}//namespace mtca4u

#endif // MTCA4U_DOOCS_PV_FACTORY_H
