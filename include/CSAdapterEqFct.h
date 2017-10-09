#ifndef CS_ADAPTER_EQ_FCT_H
#define CS_ADAPTER_EQ_FCT_H

#include <eq_fct.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariable.h>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include "DoocsUpdater.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace ChimeraTK{

class CSAdapterEqFct : public EqFct , boost::noncopyable {
 protected:
    boost::shared_ptr<ControlSystemPVManager> controlSystemPVManager_;
    int fctCode_;
    std::vector< boost::shared_ptr<D_fct> > doocsProperties_;
    void registerProcessVariablesInDoocs();
    std::vector < ChimeraTK::ProcessVariable::SharedPtr > getProcessVariablesInThisLocation();

    static bool emptyLocationVariablesHandled;
    DoocsUpdater updater_;
    
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

}// namespace ChimeraTK

#endif// CS_ADAPTER_EQ_FCT_H
