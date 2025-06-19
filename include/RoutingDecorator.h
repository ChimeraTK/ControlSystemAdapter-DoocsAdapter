// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ChimeraTK/ControlSystemAdapter/UnidirectionalProcessArray.h"
#include "ChimeraTK/NDRegisterAccessorDecorator.h"

#include <ChimeraTK/TransferElement.h>

#include <map>

namespace ChimeraTK {

  /**
   * A RoutingDecorator will be placed around all source process variables.
   * It implements either a direct pass-through of the value or a fan-out to the required number of copies.
   */
  template<typename UserType>
  class RoutingDecorator : public NDRegisterAccessorDecorator<UserType, UserType> {
   public:
    using Acc = boost::shared_ptr<ChimeraTK::NDRegisterAccessor<UserType>>;
    explicit RoutingDecorator(const Acc& target);

    using NDRegisterAccessorDecorator<UserType, UserType>::_target;
    [[nodiscard]] bool isFan() const { return _isFan; }
    /// this exchanges the target by a newly created process variable; also the readQueue is exchanged.
    void setupFan();
    void addToFan(RoutingDecorator& fan);
    auto& getSource() { return _source; }
    auto& getCopies() { return _copies; }

    [[nodiscard]] Acc decorateDeepInside([[maybe_unused]] std::function<Acc(const Acc&)> factory) override {
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

  /********************************************************************************************************************/

  class RoutingDecoratorDomain {
   public:
    /// return a new RoutingDecorator for source; creates the fan-out on first call for given source.
    TransferElement::SharedPtr add(TransferElement::SharedPtr source);
    /// For updatedElement = source of a fan-out belonging to RoutingDecoratorDomain, send out the copies and return
    /// true. If updatedElement is not source of a known fan-out do nothing and return false.
    bool send(TransferElementID updatedElement);
    bool isFanSource(TransferElementID updatedElement) { return _sourceMasters.contains(updatedElement); }

   protected:
    // maps sourceId -> FanOut
    std::map<TransferElementID, boost::shared_ptr<TransferElement>> _sourceMasters;
  };

  /********************************************************************************************************************/

  template<typename UserType>
  RoutingDecorator<UserType>::RoutingDecorator(const Acc& target)
  : NDRegisterAccessorDecorator<UserType, UserType>::NDRegisterAccessorDecorator(target) {
    // We need unique and stable TransferElementId (differently from usual decorator behavior).
    // This is required to distinguish between updates for source and updates for copies (from ReadAnyGroup).
    this->_id = TransferElementID();
    this->makeUniqueId();
  }

  template<typename UserType>
  void RoutingDecorator<UserType>::setupFan() {
    assert(!_isFan);
    // assert that we do decoration with DataConsistencyDecorator only after setupFan.
    // DataConsistencyDecorator implements a continuation of the readQueue, so the latter must not be
    // exchanged later.
    assert(!_thisIsDecorated);

    std::size_t size = this->getNumberOfSamples();
    auto pvName = this->getName() + "_fanOut_0"; // set a name just for debugging purpose
    auto [sender, receiver] = createSynchronizedProcessArray<UserType>(size, pvName);
    _source = _target;
    _copies.emplace_back(sender);
    // set receiver as our new target. this also exchanges our future_queue. But make sure to keep our id.
    auto id = this->getId();
    this->initFromTarget(receiver);
    this->_id = id;
    _isFan = true;
  }

  template<typename UserType>
  void RoutingDecorator<UserType>::addToFan(RoutingDecorator& fan) {
    assert(fan._isFan);
    std::size_t size = this->getNumberOfSamples();
    assert(fan.getNumberOfSamples() == size);
    // a name just for debugging purpose
    auto pvName = this->getName() + "_fanOut_" + std::to_string(fan._copies.size());
    auto [sender, receiver] = createSynchronizedProcessArray<UserType>(size, pvName);
    fan._copies.emplace_back(sender);
    // set receiver as our new target. this also exchanges our future_queue. But make sure to keep our id.
    auto id = this->getId();
    this->initFromTarget(receiver);
    this->_id = id;
  }

} // namespace ChimeraTK
