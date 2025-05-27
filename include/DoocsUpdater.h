// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ChimeraTK/ControlSystemAdapter/UnidirectionalProcessArray.h"
#include "ChimeraTK/NDRegisterAccessorDecorator.h"
#include "DoocsAdapter.h"

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

    /**
     * A RoutingDecorator will be placed around all source process variables.
     * It implements either a direct pass-through of the value or a fan-out to the required number of copies.
     */
    template<typename UserType>
    class RoutingDecorator : public NDRegisterAccessorDecorator<UserType, UserType> {
     public:
      using Acc = boost::shared_ptr<ChimeraTK::NDRegisterAccessor<UserType>>;
      explicit RoutingDecorator(const Acc& target)
      : NDRegisterAccessorDecorator<UserType, UserType>::NDRegisterAccessorDecorator(target) {
        // We need unique and stable TransferElementId (differently from usual decorator behavior).
        // This is required to distinguish between updates for source and updates for copies (from ReadAnyGroup).
        this->_id = TransferElementID();
        this->makeUniqueId();
        std::cout << "RoutingDecorator for " << this->getName() << " id=" << this->getId()
                  << ", targetid=" << _target->getId() << std::endl;
      }

      using NDRegisterAccessorDecorator<UserType, UserType>::_target;
      bool isFan() const { return _isFan; }
      /// this exchanges the target by a newly created process variable; also the readQueue is exchanged.
      void setupFan() {
        assert(!_isFan);
        // assert that we do decoration with DataConsistencyDecorator only after setupFan.
        // DataConsistencyDecorator implements a continuation of the readQueue, so the latter must not be
        // exchanged later.
        assert(!_thisIsDecorated);

        // TODO generalize scalar->array
        std::size_t size = _target->getNumberOfSamples();
        auto [sender, receiver] = createSynchronizedProcessArray<UserType>(size);
        _source = _target;
        _copies.emplace_back(sender);
        // set receiver as our new target. this also exchanges our future_queue. But make sure to keep our id.
        auto id = this->getId();
        this->initFromTarget(receiver);
        // TODO fix - we have a conceptual problem here:
        // DoocsAdapter first creates the DataConsistencyDecorator, by adding things to DataConsistencyGroup,
        // and then creates more variables, requiring this setupFan method.
        // But DataConsistencyDecorator relies on previously set readQueue, it uses it as basis for continuation!
        // So by exchanging the target and readQueue here, we come to late!

        // one idea how to fix it:
        // do not yet set up DataConsistencyGroups, or more generally, do not yet call
        // PropertyBase::registerVariable or similars
        // Instead, just collect the information here (in particular, that we need a fan-out)
        // Go back later and set up things via PropertyBase::registerVariable or similar.
        // TODOs
        // (a) what do we need to collect, precisely?
        // (b) from where can we go back?
        // might want to use CSAdapterEqFct::_doocsProperties, maybe extend CSAdapterEqFct::post_init
        // or DoocsAdapter::postInitEpilog jsut before doocsAdapter.updater->run();
        // alternative idea:
        // we already have some information about network created after parse; see
        // registerProcessVariablesInDoocs using propertyDescription->getSources, might be sufficient!

        this->_id = id;
        std::cout << "setupFan for " << this->getName() << " id=" << id << ", targetid=" << _target->getId()
                  << " , senderId=" << sender->getId() << std::endl;
        _isFan = true;
      }
      void addToFan(RoutingDecorator& fan) {
        assert(fan._isFan);

        // TODO generalize scalar->array
        auto [sender, receiver] = createSynchronizedProcessArray<UserType>(1);
        fan._copies.emplace_back(sender);
        // set receiver as our new target. this also exchanges our future_queue. But make sure to keep our id.
        auto id = this->getId();
        this->initFromTarget(receiver);
        this->_id = id;
        std::cout << "addToFan for " << this->getName() << " , senderId=" << sender->getId()
                  << ", targetid=" << _target->getId() << std::endl;
        // TODO check ownership: we are modifying asCopy but not taking over ownership:
        // when it is deleted, sender remains existing but becomes useless - is that fine?
      }
      auto& getSource() { return _source; }
      auto& getCopies() { return _copies; }

      [[nodiscard]] boost::shared_ptr<NDRegisterAccessor<UserType>> decorateDeepInside(
          [[maybe_unused]] std::function<boost::shared_ptr<NDRegisterAccessor<UserType>>(
              const boost::shared_ptr<NDRegisterAccessor<UserType>>&)> factory) override {
        _thisIsDecorated = true;
        // disallow that DataConsistencyDecorator would be placed inside of RoutingDecorator
        return {};
      }

     protected:
      bool _isFan = false;
      // keep track of usage: decorator was placed around this object! Note, this flag only tracks decorateDeepInside
      // mechanism.
      bool _thisIsDecorated = false;
      Acc _source;
      std::list<Acc> _copies;
    };
    struct RoutingDecoratorDomain {
      // if updated process var is source for a fan-out, generate the copies
      // We assume that all elements (source and copies) are in our ReadAnyGroup
      bool send(TransferElementID updatedElement);

      // maps sourceId -> FanOut
      std::map<TransferElementID, boost::shared_ptr<TransferElement>> _sourceMasters;
    };
    RoutingDecoratorDomain routing;

    // for readable process variables, checks if fan-out is required and returns mapped output
    // write-only process variables are handed through
    ProcessVariable::SharedPtr getMappedProcessVariable(const ChimeraTK::RegisterPath& processVariableName);

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
