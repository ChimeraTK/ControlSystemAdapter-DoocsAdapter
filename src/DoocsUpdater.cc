// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsUpdater.h"

#include "ChimeraTK/ControlSystemAdapter/UnidirectionalProcessArray.h"
#include "ChimeraTK/NDRegisterAccessorDecorator.h"

#include <ChimeraTK/ReadAnyGroup.h>

#include <unordered_set>

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
      routing.send(updatedElement);

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
    _syncThread = boost::thread([this] { updateLoop(); });
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

  ProcessVariable::SharedPtr DoocsUpdater::getMappedProcessVariable(
      const ChimeraTK::RegisterPath& processVariableName) {
    auto controlSystemPVManager = doocsAdapter.getControlSystemPVManager();
    auto pv = controlSystemPVManager->getProcessVariable(processVariableName);

    if(!pv->isReadable()) {
      return pv;
    }
    bool sourceRequiresFan = doocsAdapter.reverseMapping.at(pv->getName()).size() > 1;
    if(!sourceRequiresFan) {
      return pv;
    }
    ProcessVariable::SharedPtr ret;
    callForType(pv->getValueType(), [&](auto t) {
      using UserType = decltype(t);

      auto source = boost::dynamic_pointer_cast<ChimeraTK::NDRegisterAccessor<UserType>>(pv);
      assert(source);
      TransferElementID sourceId = source->getId();
      auto decorator = boost::make_shared<RoutingDecorator<UserType>>(source);

      // We add source to elementsToRead only if fan acually needed - check docu future_queue::when_any.
      // We don't need a doocsUpdater function for the source.
      if(!_toDoocsDescriptorMap.contains(sourceId)) {
        _elementsToRead.emplace_back(source);
        _toDoocsDescriptorMap[sourceId];
      }

      if(!routing._sourceMasters.contains(sourceId)) {
        decorator->setupFan();
        routing._sourceMasters[sourceId] = decorator;
      }

      else {
        auto sourceMaster = boost::dynamic_pointer_cast<RoutingDecorator<UserType>>(routing._sourceMasters[sourceId]);
        assert(sourceMaster);
        decorator->template addToFan(*sourceMaster);
      }
      ret = decorator;
    });
    return ret;
  }

  bool DoocsUpdater::RoutingDecoratorDomain::send(TransferElementID updatedElement) {
    auto it = _sourceMasters.find(updatedElement);
    if(it == _sourceMasters.end()) {
      return false;
    }

    bool ret;
    callForType(it->second->getValueType(), [&](auto t) {
      using UserType = decltype(t);

      auto dec = boost::dynamic_pointer_cast<RoutingDecorator<UserType>>(it->second);
      assert(dec);
      if(!dec->isFan()) {
        ret = false;
      }

      auto& source = dec->getSource();
      auto vn = source->getVersionNumber();
      assert(vn > VersionNumber{nullptr});

      unsigned nCopies = dec->getCopies().size();
      auto jt = dec->getCopies().begin();
      for(unsigned j = 0; j < nCopies - 1; ++j, ++jt) {
        auto dest = *jt;
        for(unsigned i = 0; i < dest->getNumberOfChannels(); i++) {
          dest->accessChannel(i) = source->accessChannel(i);
        }
        dest->write(vn);
      }
      // use swap for last copy
      auto dest = *jt;
      for(unsigned i = 0; i < dest->getNumberOfChannels(); i++) {
        dest->accessChannel(i).swap(source->accessChannel(i));
      }
      dest->write(vn);
      ret = true;
    });
    return ret;
  }

} // namespace ChimeraTK
