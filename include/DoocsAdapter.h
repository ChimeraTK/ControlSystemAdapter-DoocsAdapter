#ifndef _CHIMERATK_DOOCS_ADAPTER_H_
#define _CHIMERATK_DOOCS_ADAPTER_H_

#include "CSAdapterEqFct.h"
#include "DoocsPVFactory.h"

namespace ChimeraTK {

  /**
   * Base class used for all properties.
   */
  class PropertyBase : public boost::enable_shared_from_this<PropertyBase> {
   public:
    // List of other properties which need to update their DOOCS buffers when this property is written from the DOOCS
    // side. This is used to synchronise multi-mapped PVs.
    std::set<boost::shared_ptr<PropertyBase>> otherPropertiesToUpdate;

    // Update DOOCS buffer from PVs. The given transferElementId shall be used only for checking consistency with the
    // DataConsistencyGroup. {} will be passed if the update is coming from another property (hence the update will
    // only be taken with data matching set to none, which is the expected behaviour).
    virtual void updateDoocsBuffer(const TransferElementID& transferElementId) = 0;

    virtual EqFct* getEqFct() = 0;
  };

  /** The main adapter class. With this tool the EqFct should shrink to about 4
   * lines of code (plus boiler plate).
   */
  class DoocsAdapter {
   public:
    DoocsAdapter();
    boost::shared_ptr<DevicePVManager> const& getDevicePVManager() const;
    boost::shared_ptr<ControlSystemPVManager> const& getControlSystemPVManager() const;

    boost::shared_ptr<DoocsUpdater> updater;

    // Function to be called in all auto_init() implementations, to initialise otherPropertiesToUpdate lists in all
    // properties. This needs to be done after all locations have been created but before the properties get their
    // initial values from the config file. DOOCS seems not to provide any hook at that point... This function will only
    // perform an action when called for the first time.
    void before_auto_init();

    // An atomic bool which is set true in post_init_epilog to indicate that doocs
    // is ready. Only used in testing.
    static std::atomic<bool> isInitialised;

    // A convenience function to wait until the adapter is initialised.
    static void waitUntilInitialised();

    // Function used by the property implementations to decide whether to print a "data loss" warning
    static bool checkPrintDataLossWarning(size_t counter);

    // stores list of writable PVs which are mapped to multiple properties
    // Note: this is cleared in post_init_epilog() to save memory.
    std::map<std::string, std::set<boost::shared_ptr<PropertyBase>>> writeableVariablesWithMultipleProperties;

   protected:
    boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager;
    boost::shared_ptr<DevicePVManager> _devicePVManager;

    // flag whether before_auto_init() has already been called.
    bool before_auto_init_called{false};
  };

} // namespace ChimeraTK

extern ChimeraTK::DoocsAdapter doocsAdapter;

#endif // _CHIMERATK_DOOCS_ADAPTER_H_
