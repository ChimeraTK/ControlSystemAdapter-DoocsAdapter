// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ChimeraTK/TypeChangingDecorator.h"
#include "DoocsAdapter.h"
#include "RoutingDecorator.h"

#include <ChimeraTK/TransferElement.h>
#include <ChimeraTK/TransferElementAbstractor.h>

#include <boost/noncopyable.hpp>

#include <eq_fct.h>

#include <map>
#include <utility>

namespace ChimeraTK {

  /** A class to synchronise DeviceToControlSystem variable to Doocs.
   *  It contains a list of TransferElements and a thread which is monitoring them
   * for updates. The thread has to be started with the run() functions, which
   * returns immediately when the thread is started, and (FIXME can be stopped by
   * the stop() function which returns after the thread has been joined). This
   * happens latest in the destructor.
   */
  class DoocsUpdater : public boost::noncopyable {
   public:
    // depends on global doocsAdapter instance only via default parameter value (so unit tests can be independent of
    // the doocsAdapter instance)
    DoocsUpdater(boost::shared_ptr<ControlSystemPVManager> csManager = doocsAdapter.getControlSystemPVManager())
    : _controlSystemPVManager(std::move(csManager)) {}
    ~DoocsUpdater();
    void update(); // Update all variables once. This is a convenience function
                   // for testing.

    void updateLoop(); // Endless loop with interruption point around the update
                       // function.

    void run();
    void stop();

    /**
     *  Add a variable to be updated. Together with the TransferElementAbstractor
     *  pointing to the ChimeraTK::ProcessArray, the EqFct* to obtain the lock for
     *  and a function to be called which executes the actual update should be
     *  specified. The lock is held while the updaterFunction is called, so it must
     *  neither obtained nor freed within the updaterFunction.
     *  eq_fct and updateFunction can be empty, in case the variable is used by only by other updaterFunctions.
     */
    void addVariable(ChimeraTK::TransferElementAbstractor variable, EqFct* eq_fct = nullptr,
        const std::function<void()>& updaterFunction = {});

    const std::list<ChimeraTK::TransferElementAbstractor>& getElementsToRead() { return _elementsToRead; }

    RoutingDecoratorDomain routing;

    template<typename UserType>
    boost::shared_ptr<NDRegisterAccessor<UserType>> getMappedProcessVariable(
        const ChimeraTK::RegisterPath& processVariableName,
        DecoratorType decoratorType = DecoratorType::C_style_conversion);

    // For readable process variables, checks if fan-out is required and returns mapped output.
    // Write-only process variables are handed through.
    TransferElement::SharedPtr getMappedProcessVariableUnTyped(const ChimeraTK::RegisterPath& processVariableName);
    void setPvNamesWithFan(std::set<std::string> pvNamesWithFan) { _pvNamesWithFan = std::move(pvNamesWithFan); }

   protected:
    boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager;
    std::set<std::string> _pvNamesWithFan;
    std::list<ChimeraTK::TransferElementAbstractor> _elementsToRead;
    boost::thread _syncThread; // we have to use boost thread to use interruption points

    // Struct used to aggregate the information needed in the updateLoop when an update is received from the
    // application.
    struct ToDoocsUpdateDescriptor {
      std::vector<std::function<void()>> updateFunctions;
      std::vector<EqFct*> locations;
    };
    std::map<ChimeraTK::TransferElementID, ToDoocsUpdateDescriptor> _toDoocsDescriptorMap;
  };

  /********************************************************************************************************************/

  template<typename UserType>
  boost::shared_ptr<NDRegisterAccessor<UserType>> DoocsUpdater::getMappedProcessVariable(
      const RegisterPath& processVariableName, DecoratorType decoratorType) {
    auto pv = getMappedProcessVariableUnTyped(processVariableName);
    if(typeid(UserType) == pv->getValueType()) {
      return boost::dynamic_pointer_cast<NDRegisterAccessor<UserType>>(pv);
    }

    if constexpr(std::is_same_v<UserType, std::string>) {
      // FIXME: implement TypeChangigDecorator for std::string
      throw std::invalid_argument(
          std::string("DoocsUpdater::getMappedProcessVariable: processArray is not of string type: ") + pv->getName());
    }
    else {
      return getTypeChangingDecorator<UserType>(pv, decoratorType);
    }
  }
} // namespace ChimeraTK
