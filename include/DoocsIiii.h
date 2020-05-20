#pragma once

#include <D_xy.h>
#include <boost/noncopyable.hpp>

#include <ChimeraTK/NDRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>

class EqFct;

namespace ChimeraTK {
  class DoocsUpdater;

  class DoocsIiii : public D_iiii, public boost::noncopyable {
   public:
    // Constructor with history enabled
    DoocsIiii(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i2Value,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i3Value,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i4Value, DoocsUpdater& updater);

    // Constructor without history
    DoocsIiii(std::string const& doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i1Value,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i2Value,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i3Value,
        boost::shared_ptr<NDRegisterAccessor<int>> const& i4Value, DoocsUpdater& updater);

    void set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) override;
    void auto_init(void) override;

    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource);
    void publishZeroMQ() { _publishZMQ = true; }
    DataConsistencyGroup _consistencyGroup;

   protected:
    void updateAppToDoocs(TransferElementID& elementId);
    void sendToApplication();
    void registerVariable(const ChimeraTK::TransferElementAbstractor& var);
    void registerIiiiSources();
    void checkSourceConsistency();

    boost::shared_ptr<NDRegisterAccessor<int>> _i1Value;
    boost::shared_ptr<NDRegisterAccessor<int>> _i2Value;
    boost::shared_ptr<NDRegisterAccessor<int>> _i3Value;
    boost::shared_ptr<NDRegisterAccessor<int>> _i4Value;
    DoocsUpdater& _updater;
    EqFct* _eqFct;
    bool _publishZMQ{false};

    bool isWriteable;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
  };
} // namespace ChimeraTK
