// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ChimeraTK/ControlSystemAdapter/UnidirectionalProcessArray.h"
#include "ChimeraTK/NDRegisterAccessorDecorator.h"

#include <ChimeraTK/TransferElement.h>
#include <ChimeraTK/TransferElementAbstractor.h>

#include <boost/noncopyable.hpp>

#include <eq_fct.h>

#include <map>
#include <unordered_map>

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
    ~DoocsUpdater();
    void update(); // Update all variables once. This is a convenience function
                   // for testing.

    void updateLoop(); // Endless loop with interruption point around the update
                       // function.

    void run();
    void stop();

    // Add a variable to be updated. Together with the TransferElementAbstractor
    // pointing to the ChimeraTK::ProcessArray, the EqFct* to obtain the lock for
    // and a function to be called which executes the actual update should be
    // specified. The lock is held while the updaterFunction is called, so it must
    // neither obtained nor freed within the updaterFunction.
    void addVariable(
        ChimeraTK::TransferElementAbstractor variable, EqFct* eq_fct, const std::function<void()>& updaterFunction);

    const std::list<ChimeraTK::TransferElementAbstractor>& getElementsToRead() { return _elementsToRead; }

    // TODO replace this by template argument, in order to generalize
    using MPUserType = int64_t;
    using MPAcc = boost::shared_ptr<ChimeraTK::NDRegisterAccessor<MPUserType>>;
    /**
     * A RoutingDecorator will be placed around all source process variables.
     * It implements either a direct pass-through of the value or a fan-out to the required number of copies.
     */
    class RoutingDecorator : public NDRegisterAccessorDecorator<MPUserType, MPUserType> {
     public:
      using NDRegisterAccessorDecorator<MPUserType, MPUserType>::NDRegisterAccessorDecorator;
      using NDRegisterAccessorDecorator<MPUserType, MPUserType>::_target;
      bool isFan() const { return _isFan; }
      void setupFan() {
        std::cout << "setupFan for " << getName() << std::endl;
        auto [sender, receiver] = createSynchronizedProcessArray<MPUserType>(1);
        _source = _target;
        _copies.emplace_back(sender);
        _target = receiver;
        _isFan = true;
      }
      auto& getSource() { return _source; }
      auto& getCopies() { return _copies; }

     protected:
      bool _isFan = false;
      MPAcc _source;
      std::list<MPAcc> _copies;
    };
    std::map<TransferElementID, boost::shared_ptr<RoutingDecorator>> _sourceMasters;
    MPAcc map(const MPAcc& source);

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> copyOfMacroPulseSource(
        const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>& macroPulseNumberSource);

   protected:
    std::list<ChimeraTK::TransferElementAbstractor> _elementsToRead;
    boost::thread _syncThread; // we have to use boost thread to use interruption points

    // Struct used to aggregate the information needed in the updateLoop when an update is received from the
    // application.
    struct ToDoocsUpdateDescriptor {
      std::vector<std::function<void()>> updateFunctions;
      std::vector<EqFct*> locations;
      std::set<boost::shared_ptr<ChimeraTK::TransferElement>> additionalTransferElements;
    };
    std::map<ChimeraTK::TransferElementID, ToDoocsUpdateDescriptor> _toDoocsDescriptorMap;
  };
} // namespace ChimeraTK
