#ifndef MTCA4U_DOOCS_PV_MANAGER_H
#define MTCA4U_DOOCS_PV_MANAGER_H

#include <ControlSystemAdapter/ControlSystemPVManager.h>
#include <ControlSystemAdapter/ControlSystemProcessVariable.h>

namespace mtca4u{

  class DoocsPVManager{
  public:
    DoocsPVManager( boost::shared_ptr<ControlSystemPVManager> pvManager );

    template<class T > 
      typename ControlSystemProcessScalar< T >::SharedPtr getProcessScalar (const std::string &processVariableName) const;

    template<class T > 
      typename ControlSystemProcessArray< T >::SharedPtr getProcessArray (const std::string &processVariableName) const;
//
//    std::map< std::string, ControlSystemProcessVariable::SharedPtr > getAllProcessVariables () const;
  
    void synchronize();
    void setModified(ControlSystemProcessVariable::SharedPtr & processVariable);

  private:
    std::list< ControlSystemProcessVariable::SharedPtr > _toDeviceProcessVariables;
    std::list< ControlSystemProcessVariable::SharedPtr > _fromDeviceProcessVariables;
    boost::shared_ptr<ControlSystemPVManager> _pvManager;    
  };
}// namespace mtca4u

#endif // MTCA4U_DOOCS_PV_MANAGER_H
