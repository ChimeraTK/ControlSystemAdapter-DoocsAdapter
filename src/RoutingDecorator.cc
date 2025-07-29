// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RoutingDecorator.h"

namespace ChimeraTK {

  TransferElement::SharedPtr RoutingDecoratorDomain::add(TransferElement::SharedPtr source) {
    TransferElement::SharedPtr ret;
    callForType(source->getValueType(), [&](auto t) {
      using UserType = decltype(t);

      auto sourceWithType = boost::dynamic_pointer_cast<ChimeraTK::NDRegisterAccessor<UserType>>(source);
      assert(sourceWithType);
      TransferElementID sourceId = source->getId();
      auto decorator = boost::make_shared<RoutingDecorator<UserType>>(sourceWithType);

      if(!_sourceMasters.contains(sourceId)) {
        decorator->setupFan();
        _sourceMasters[sourceId] = decorator;
      }

      else {
        auto sourceMaster = boost::dynamic_pointer_cast<RoutingDecorator<UserType>>(_sourceMasters[sourceId]);
        assert(sourceMaster);
        decorator->addToFan(*sourceMaster);
      }
      ret = decorator;
    });
    return ret;
  }

  bool RoutingDecoratorDomain::send(TransferElementID updatedElement) {
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
        // currently we support only 1 channel
        dest->accessChannel(0) = source->accessChannel(0);
        dest->write(vn);
      }
      // use swap for last copy
      auto dest = *jt;
      dest->accessChannel(0).swap(source->accessChannel(0));
      dest->write(vn);
      ret = true;
    });
    return ret;
  }

} // namespace ChimeraTK
