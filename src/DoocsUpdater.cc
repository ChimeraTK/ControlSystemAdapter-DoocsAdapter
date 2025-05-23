// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsUpdater.h"

#include "ChimeraTK/ControlSystemAdapter/UnidirectionalProcessArray.h"
#include "ChimeraTK/NDRegisterAccessorDecorator.h"

#include <ChimeraTK/ReadAnyGroup.h>

#include <unordered_set>

// TODO discuss - this is code duplication, copied from ApplicationCore/Utilities.
// maybe move to DeviceAccess/Utilities?
// But function would also be useful for doocs-server-test-helper which does not depend on DeviceAccess!
static void setThreadName(const std::string& name) {
#if defined(__linux__)
  pthread_setname_np(pthread_self(), name.substr(0, std::min<std::string::size_type>(name.length(), 15)).c_str());
#elif defined(__APPLE__)
  pthread_setname_np(name.substr(0, std::min<std::string::size_type>(name.length(), 15)).c_str());
#endif
}

namespace ChimeraTK {

  void DoocsUpdater::addVariable(
      TransferElementAbstractor variable, EqFct* eq_fct, const std::function<void()>& updaterFunction) {
    // Don't add the transfer element twice into the list of elements to read (i.e. later into the ReadAnyGroup).
    // To check if there is such an element we use the map with the lookup table
    // which has a search function, instead of manually looking at the elements in
    // the list and compare the ID.
    if(_toDoocsDescriptorMap.find(variable.getId()) == _toDoocsDescriptorMap.end()) {
      _elementsToRead.push_back(variable);
    }
    else {
      _toDoocsDescriptorMap[variable.getId()].additionalTransferElements.insert(variable.getHighLevelImplElement());
    }

    _toDoocsDescriptorMap[variable.getId()].updateFunctions.push_back(updaterFunction);
    _toDoocsDescriptorMap[variable.getId()].locations.push_back(eq_fct);
  }

  boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> DoocsUpdater::copyOfMacroPulseSource(
      const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>>& macroPulseNumberSource) {
    // TODO fix:
    // we are facing the problem that macroPulse source can be used from several DataConsistencyGroups and
    // MatchingMode::historized. This is not allowed. As a workaround, we need a kind of fan-out to separate
    // TransferElements for each macroPulse source instance. The fan-out can be realized by a thread listening on the
    // macroPulse source and writing to the needed copies. The needed copies should be inserted into _elementsToRead and
    // be used as macroPulse sources.

    return map(macroPulseNumberSource);
  }

  void DoocsUpdater::update() {
    for(auto& transferElem : _elementsToRead) {
      if(transferElem.readLatest()) {
        for(auto& updaterFunction : _toDoocsDescriptorMap[transferElem.getId()].updateFunctions) {
          updaterFunction();
        }
      }
    }
  }

  void DoocsUpdater::updateLoop() {
    if(_elementsToRead.empty()) {
      return;
    }

    std::unordered_set<EqFct*> locationsToLock;

    // preallocate locationsToLock to avoid breaking "realtime" behaivour
    size_t nMaxLocationsToLock = 0;
    for(auto& pair : _toDoocsDescriptorMap) {
      nMaxLocationsToLock = std::max(nMaxLocationsToLock, pair.second.locations.size());
    }
    locationsToLock.reserve(nMaxLocationsToLock);

    // TODO debug:with serverTestDataMatching.cpp, _elementsToRead[1] is
    // DataConsistencyDecorator_historized(RoutingDecorator_isFan(UniDirProcessVariable_receiving(macroPulseId='/INT/FROM_DEVICE_SCALAR')
    // and it has already here, before its sending side was written to, notifyerQueue_previousData>0 set!
    // Thats strange, who caused that/pushed to future_queue?
    ReadAnyGroup group(_elementsToRead.begin(), _elementsToRead.end());

    // Call preRead for all TEs on additional transfer elements. waitAny() is doing this for all elements in the
    // ReadAnyGroup. Unnecessary calls to preRead() are anyway ignored and merely pose a performance issue. For large
    // servers, the performance impact is significant, hence we keep track of the TEs which need to be called.
    for(auto& pair : _toDoocsDescriptorMap) {
      for(const auto& elem : pair.second.additionalTransferElements) {
        elem->preRead(ChimeraTK::TransferType::read);
      }
    }

    while(true) {
      // Wait until any variable got an update
      auto notification = group.waitAny();
      auto updatedElement = notification.getId();
      auto& descriptor = _toDoocsDescriptorMap[updatedElement];

      // Gather all involved locations in a unique set
      for(auto& location : descriptor.locations) {
        if(locationsToLock.insert(location).second) {
          location->lock();
        }
      }
      // Complete the read transfer of the process variable
      if(!notification.accept()) {
        // discard void notification (e.g. because of inconsistent data)
        // Unlock all involved locations
        for(const auto& location : locationsToLock) {
          location->unlock();
        }
        locationsToLock.clear();
        continue;
      }

      // TODO debug: with var /DOUBLE/CONSTANT_ARRAY we get an update
      // with VersionNumber=0, Validity=false. Why, what does it mean?
      // Also, why is it accepted by HistorizedMatcher as a match?
      // Anyway, we get failed assertion about update with VersionNumber=0!
      {
        auto te = notification.getTransferElement();
        std::cout << "update: " << te.getName() << " " << te.getVersionNumber() << std::endl;
        assert(te.getVersionNumber() > VersionNumber{0});
      }

      // if updated process var is source for a fan-out, generate the copies
      // We assume that all elements (source and copies) are in our ReadAnyGroup
      auto it = _sourceMasters.find(updatedElement);
      if(it != _sourceMasters.end() && it->second->isFan()) {
        // TODO refactor -> member function
        RoutingDecorator& dec = *it->second;
        auto& source = dec.getSource();
        auto vn = source->getVersionNumber();
        assert(vn > VersionNumber{0});

        for(auto& dest : dec.getCopies()) {
          // TODO optimize in order to use swap for last copy
          dest->accessData(0) = source->accessData(0);
          dest->write(vn);
        }
      }

      // Call postRead for all TEs on _toDoocsAdditionalTransferElementsMap for the updated ID
      for(const auto& elem : descriptor.additionalTransferElements) {
        elem->postRead(ChimeraTK::TransferType::read, true);
      }

      // TODO fix - I think I missed a problem here:
      // the DataConsistencyGroup of updatedElement as source, does not imply that updates for the copies are
      // already present. But updaterFunction must only be called when corresponding updates are there.
      // What about ids of source and copies, I guess they are different?

      // Call all updater functions
      for(auto& updaterFunction : descriptor.updateFunctions) {
        updaterFunction();
      }

      // Unlock all involved locations
      for(const auto& location : locationsToLock) {
        location->unlock();
      }
      locationsToLock.clear();

      // Call preRead for all TEs on _toDoocsAdditionalTransferElementsMap for the updated ID
      for(const auto& elem : descriptor.additionalTransferElements) {
        elem->preRead(ChimeraTK::TransferType::read);
      }

      // Allow shutting down this thread...
      boost::this_thread::interruption_point();
    }
  }

  void DoocsUpdater::run() {
    _syncThread = boost::thread([this] {
      setThreadName("DoocsUpdater");
      updateLoop();
    });
  }

  void DoocsUpdater::stop() {
    _syncThread.interrupt();
    for(auto& var : _elementsToRead) {
      var.getHighLevelImplElement()->interrupt();
    }
    _syncThread.join();
  }

  DoocsUpdater::~DoocsUpdater() {
    stop();
  }

  DoocsUpdater::MPAcc DoocsUpdater::map(const DoocsUpdater::MPAcc& source) {
    // TODO fix several problems
    // - we must generalize the fan-out to handle not just macro pulses. also relevant e.g. for spectrum inputs
    // - currently we have a problem if source is mapped to DOOCS; then it appears already in ReadAnyGroup there,
    //   and cannot be put into another ReadAnyGroup
    // => we must use our mapping/fan-out on all process vars (whether mapped directly or used in DataConsistencyGroup)
    // - We should also try to get rid of the thread and the fan-out in cases where there is a 1-to-1 mapping;
    //   unclear how to find out about this in advance.
    //   An idea (martin) would be to put a Decorator around all used process vars and change Decorator target
    //   later, depending on requirements: either, target is set directly to process var, or to fan-out output.
    // - Discussion about thread or not:
    //   We could try to eliminate fan-out thread altogether, idea would be to put only sources into ReadAnyGroup,
    //   and handle updates for them by fanOut.read() which would write to sender and then (non-blocking)read
    //   receivers, or more precisely, Decorators put around receivers.

    if(!_sourceMasters.contains(source->getId())) {
      auto decorator = boost::make_shared<RoutingDecorator>(source);
      _sourceMasters[source->getId()] = decorator;
      return decorator;
    }
    auto& sourceMaster = _sourceMasters[source->getId()];
    // only first time we get here, we must modify sourceMaster to point to newly created receiver
    if(!sourceMaster->isFan()) {
      sourceMaster->setupFan();
      // We add source to elementsToRead only if fan acually needed - check docu future_queue::when_any.
      // We don't need a doocsUpdater function for the source.
      if(!_toDoocsDescriptorMap.contains(source->getId())) {
        _elementsToRead.emplace_back(source);
        _toDoocsDescriptorMap[source->getId()];
      }
    }
    auto decorator = boost::make_shared<RoutingDecorator>(source);
    decorator->addToFan(*sourceMaster);
    return decorator;
  }

} // namespace ChimeraTK
