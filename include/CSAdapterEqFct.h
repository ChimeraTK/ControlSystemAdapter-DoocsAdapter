#ifndef CS_ADAPTER_EQ_FCT_H
#define CS_ADAPTER_EQ_FCT_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <eq_fct.h>

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessVariable.h>

#include "DoocsUpdater.h"
#include "PropertyDescription.h"

#include "StatusHandler.h"

namespace ChimeraTK {

  template<typename DOOCS_T, typename DOOCS_PRIMITIVE_T>
  class DoocsProcessArray;

  class CSAdapterEqFct : public EqFct, boost::noncopyable {
   protected:
    boost::shared_ptr<ControlSystemPVManager> controlSystemPVManager_;
    int fctCode_;
    std::map<std::shared_ptr<ChimeraTK::PropertyDescription>, boost::shared_ptr<D_fct>> doocsProperties_;
    void registerProcessVariablesInDoocs();
    std::vector<ChimeraTK::ProcessVariable::SharedPtr> getProcessVariablesInThisLocation();

    static bool emptyLocationVariablesHandled;
    boost::shared_ptr<DoocsUpdater> updater_;

    boost::shared_ptr<StatusHandler> statusHandler_;

   public:
    // The fctName (location name ) is usually coming from the config file and
    // should be left empty. Only for testing without actually running a DOOCS
    // server you need it.
    CSAdapterEqFct(int fctCode,
        boost::shared_ptr<ControlSystemPVManager> const& controlSystemPVManager,
        boost::shared_ptr<DoocsUpdater> const& updater,
        std::string fctName = std::string());
    ~CSAdapterEqFct();

    void init() override;
    void post_init() override;
    int fct_code() override;

    template<class ValueType>
    void saveArray(D_fct* p);
    int write(std::ostream &fprt) override;
  };

} // namespace ChimeraTK

#endif // CS_ADAPTER_EQ_FCT_H
