#ifndef _CHIMERATK_DOOCS_ADAPTER_H_
#define _CHIMERATK_DOOCS_ADAPTER_H_

#include "CSAdapterEqFct.h"
#include "DoocsAdapter.h"
#include "DoocsPVFactory.h"

namespace ChimeraTK {

  /** The main adapter class. With this tool the EqFct should shrink to about 4
   * lines of code (plus boiler plate).
   */
  class DoocsAdapter {
   public:
    DoocsAdapter();
    boost::shared_ptr<DevicePVManager> const& getDevicePVManager() const;
    boost::shared_ptr<ControlSystemPVManager> const& getControlSystemPVManager() const;

    boost::shared_ptr<DoocsUpdater> updater;

    // An atomic bool which is set true in post_init_epilog to indicate that doocs
    // is ready. Only used in testing.
    static std::atomic<bool> isInitialised;

    // A convenience function to wait until the adapter is initialised.
    static void waitUntilInitialised();

    // Function used by the property implementations to decide whether to print a "data loss" warning
    static bool checkPrintDataLossWarning(size_t counter);

   protected:
    boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager;
    boost::shared_ptr<DevicePVManager> _devicePVManager;
  };

} // namespace ChimeraTK

#endif // _CHIMERATK_DOOCS_ADAPTER_H_
