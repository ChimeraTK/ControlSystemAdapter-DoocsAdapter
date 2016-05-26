#ifndef CS_ADAPTER_EQ_FCT_H
#define CS_ADAPTER_EQ_FCT_H

#include <eq_fct.h>
#include <ControlSystemAdapter/ProcessVariable.h>
#include <ControlSystemAdapter/ControlSystemSynchronizationUtility.h>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace mtca4u{

class CSAdapterEqFct : public EqFct , boost::noncopyable {
 protected:
    boost::shared_ptr<ControlSystemPVManager> controlSystemPVManager_;
    boost::shared_ptr< ControlSystemSynchronizationUtility > syncUtility_;
    int fctCode_;
    std::vector< boost::shared_ptr<D_fct> > doocsProperties_;
    std::vector< boost::shared_ptr<mtca4u::ProcessVariable> > mtca4uReceivers_;
    void registerProcessVariablesInDoocs();
    std::vector < mtca4u::ProcessVariable::SharedPtr > getProcessVariablesInThisLocation();

    static bool emptyLocationVariablesHandled;
    
 public:
    // The fctName (location name ) is usually coming from the config file and
    // should be left empty. Only for testing without actually running a DOOCS server
    // you need it.
    CSAdapterEqFct(int fctCode,
		   boost::shared_ptr<ControlSystemPVManager> const & controlSystemPVManager,
		   std::string fctName = std::string());

    void init();
    void update();
    int fct_code();
};

}// namespace mtca4u

#endif// CS_ADAPTER_EQ_FCT_H
