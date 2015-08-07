#ifndef MTCA4U_DOOCS_PV_MANAGER_H
#define MTCA4U_DOOCS_PV_MANAGER_H

#include <ControlSystemAdapter/ControlSystemPVManager.h>
#include <ControlSystemAdapter/ControlSystemProcessVariable.h>

namespace mtca4u{

  class DoocsPVManager{
  public:
    DoocsPVManager( boost::shared_ptr<ControlSystemPVManager> pvManager );

    template<class T > 
    typename ControlSystemProcessScalar< T >::SharedPtr getProcessScalar (
      const std::string &processVariableName) const{
      return _pvManager->getProcessScalar<T>(processVariableName);
    }

    template<class T > 
    typename ControlSystemProcessArray< T >::SharedPtr getProcessArray (
      const std::string &processVariableName) const{
      return _pvManager->getProcessArray<T>(processVariableName);
    }
//
//    std::map< std::string, ControlSystemProcessVariable::SharedPtr > getAllProcessVariables () const;
  
    void synchronize();
    void setModified(ControlSystemProcessVariable::SharedPtr & processVariable);

    // The internal variables are not declared private but protected so
    // a test can derrive from the manager and access them.
  protected:
    std::list< ControlSystemProcessVariable::SharedPtr > _toDeviceProcessVariables;
    boost::shared_ptr<ControlSystemPVManager> _pvManager;    
 };
}// namespace mtca4u

#endif // MTCA4U_DOOCS_PV_MANAGER_H
