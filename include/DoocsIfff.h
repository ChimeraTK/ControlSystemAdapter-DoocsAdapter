#pragma once

#include <D_xy.h>
#include <boost/noncopyable.hpp>

#include <ChimeraTK/NDRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>

class EqFct;

namespace ChimeraTK {
  class DoocsUpdater;

  class DoocsIfff : public D_ifff, public boost::noncopyable {
   public:
    // Constructor with history enabled
    DoocsIfff(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
        boost::shared_ptr<NDRegisterAccessor<float>> const& f1Value,
        boost::shared_ptr<NDRegisterAccessor<float>> const& f2Value,
        boost::shared_ptr<NDRegisterAccessor<float>> const& f3Value, DoocsUpdater& updater);

    // Constructor without history
    DoocsIfff(std::string const& doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
        boost::shared_ptr<NDRegisterAccessor<float>> const& f1Value,
        boost::shared_ptr<NDRegisterAccessor<float>> const& f2Value,
        boost::shared_ptr<NDRegisterAccessor<float>> const& f3Value, DoocsUpdater& updater);

    void set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) override;
    void auto_init(void) override;

   protected:
    void updateAppToDoocs(TransferElementID& elementId);
    void sendToApplication();

    DataConsistencyGroup _consistencyGroup;
    boost::shared_ptr<NDRegisterAccessor<int>> _i1Value;
    boost::shared_ptr<NDRegisterAccessor<float>> _f1Value;
    boost::shared_ptr<NDRegisterAccessor<float>> _f2Value;
    boost::shared_ptr<NDRegisterAccessor<float>> _f3Value;

    bool isWriteable;
  };
} // namespace ChimeraTK
