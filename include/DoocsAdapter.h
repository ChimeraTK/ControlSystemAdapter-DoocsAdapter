// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "PropertyBase.h"

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h>

#include <doocs/Server.h>

#include <eq_fct.h>

namespace ChimeraTK {

  /** The main adapter class. With this tool the EqFct should shrink to about 4
   * lines of code (plus boiler plate).
   */
  class DoocsAdapter {
   public:
    DoocsAdapter();
    [[nodiscard]] boost::shared_ptr<DevicePVManager> const& getDevicePVManager() const;
    [[nodiscard]] boost::shared_ptr<ControlSystemPVManager> const& getControlSystemPVManager() const;

    boost::shared_ptr<DoocsUpdater> updater;

    // Function to be called in all auto_init() implementations, to initialise otherPropertiesToUpdate lists in all
    // properties. This needs to be done after all locations have been created but before the properties get their
    // initial values from the config file. DOOCS seems not to provide any hook at that point... This function will only
    // perform an action when called for the first time.
    void beforeAutoInit();

    // An atomic bool which is set true in post_init_epilog to indicate that doocs
    // is ready. Only used in testing.
    static std::atomic<bool> isInitialised;

    // A convenience function to wait until the adapter is initialised.
    static void waitUntilInitialised();

    // Function used by the property implementations to decide whether to print a "data loss" warning
    static bool checkPrintDataLossWarning(size_t counter);

    // stores list of writable PVs which are mapped to multiple properties
    // Note: this is cleared in post_init_epilog() to save memory.
    std::map<std::string, std::set<boost::weak_ptr<PropertyBase>>> writeableVariablesWithMultipleProperties;

    // create the doocs::Server object
    static std::unique_ptr<doocs::Server> createServer();

    boost::shared_ptr<ControlSystemPVManager> getControlSystemPVManager() { return _controlSystemPVManager; }

    static void eqInitProlog();
    static void postInitEpilog();
    static void eqCancel();

   protected:
    boost::shared_ptr<DevicePVManager> _devicePVManager;
    boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager;

    // flag whether before_auto_init() has already been called.
    bool _before_auto_init_called{false};
  };

} // namespace ChimeraTK

extern ChimeraTK::DoocsAdapter doocsAdapter;
