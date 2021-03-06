#pragma once

#include <D_xy.h>
#include <boost/noncopyable.hpp>

#include <ChimeraTK/NDRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>

class EqFct;

namespace ChimeraTK {
  class DoocsUpdater;

  class DoocsXy : public D_xy, public boost::noncopyable {
   public:
    DoocsXy(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<NDRegisterAccessor<float>> const& xValues,
        boost::shared_ptr<NDRegisterAccessor<float>> const& yValues, DoocsUpdater& updater);

   protected:
    void updateValues(TransferElementID& elementId);

    DataConsistencyGroup _consistencyGroup;
    boost::shared_ptr<NDRegisterAccessor<float>> _xValues;
    boost::shared_ptr<NDRegisterAccessor<float>> _yValues;
  };
} // namespace ChimeraTK
