#ifndef __DOOCS_PROCESS_SCALAR_H__
#define __DOOCS_PROCESS_SCALAR_H__

#include "DoocsUpdater.h"
#include "DoocsAdapter.h"
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <boost/shared_ptr.hpp>
#include <chrono>
#include <d_fct.h>
#include <eq_fct.h>
#include <string>

namespace ChimeraTK {

  /** The DoocsProcessScalar has three template parameters:
   *  \li \c T, The primitive value type of the ChimeraTK process variable
   *  \li \c DOOCS_T, The Doocs type which is used
   */
  template<typename T, typename DOOCS_T>
  class DoocsProcessScalar : public DOOCS_T, public boost::noncopyable {
   public:
    void updateDoocsBuffer(TransferElementID transferElementId) {
      // FIXME: A first  implementation is checking the data consistency here. Later this should be
      // before calling this function because calling this function through a function pointer is
      // comparatively expensive.
      // Only check the consistency group if there is a macro pulse number associated.
      if(_macroPulseNumberSource && !_consistencyGroup.update(transferElementId)) {
        // data is not consistent (yet). Don't update the Doocs buffer.
        // check if this will now throw away data and generate a warning
        if(transferElementId == _processScalar->getId()) {
          if(!_doocsSuccessfullyUpdated) {
            ++_nDataLossWarnings;
            if(DoocsAdapter::checkPrintDataLossWarning(_nDataLossWarnings)) {
              std::cout << "WARNING: Data loss in scalar property " << _eqFct->name() << "/" << this->basename()
                        << " due to failed data matching between value and macro pulse number (repeated "
                        << _nDataLossWarnings << " times)." << std::endl;
            }
          }
        }
        _doocsSuccessfullyUpdated = false;
        return;
      }
      _doocsSuccessfullyUpdated = true;

      // Note: we already own the location lock by specification of the
      // DoocsUpdater
      auto data = _processScalar->accessData(0);

      bool storeInHistory = true;
      auto archiverStatus = ArchiveStatus::sts_ok;
      if(_processScalar->dataValidity() != ChimeraTK::DataValidity::ok) {
        if(this->d_error()) // data are alredy invalid, do not store in history
          storeInHistory = false;

        archiverStatus = ArchiveStatus::sts_err;
        //set data invalid in DOOCS for current data
        this->d_error(stale_data);
      }
      else {
        this->d_error(no_error);
      }

      // Convert time stamp from version number in Unix time (seconds and microseconds).
      // Note that epoch of std::chrono::system_time might be different from Unix time, and Unix time omits leap seconds
      // and hence is not the duration since the epoch! We have to convert to time_t and then find out the microseconds.
      doocs::Timestamp timestamp(_processScalar->getVersionNumber().getTime());
      auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
      auto seconds = sinceEpoch.seconds;
      auto microseconds = sinceEpoch.microseconds;

      // update global time stamp of DOOCS, but only if our time stamp is newer
      if(get_global_timestamp() < timestamp) {
        set_global_timestamp(timestamp);
      }

      // we must not call set_and_archive if there is no history (otherwise it
      // will be activated), but we have to if it is there. -> Abstraction,
      // please!
      //
      // We should also checked if data should be stored (flag storeInHistory). Invalid data should NOT be stored except first invalid data point.
      // (https://github.com/ChimeraTK/ControlSystemAdapter-DoocsAdapter/issues/40)
      if(this->get_histPointer() && storeInHistory) {
        // Set eventId
        doocs::EventId eventId;
        if(_macroPulseNumberSource) eventId = doocs::EventId(_macroPulseNumberSource->accessData(0));

        // The timestamp we give with set_and_archive is for the archiver only.
        this->set_and_archive(data, archiverStatus, timestamp, eventId);
      }
      else {
        this->set_value(data);
      }

      // We must set the timestamp again so it is correctly attached to the variable. set_and_archive does not to it.
      // This must happen after set_and_archive, otherwise the global time stamp is taken.
      this->set_tmstmp(seconds, microseconds);
      if(_macroPulseNumberSource) this->set_mpnum(_macroPulseNumberSource->accessData(0));

      // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
      if(_publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
        dmsg_info info;
        memset(&info, 0, sizeof(info));
        info.sec = seconds;
        info.usec = microseconds;
        if(_macroPulseNumberSource != nullptr) {
          info.ident = _macroPulseNumberSource->accessData(0);
        }
        else {
          info.ident = 0;
        }
        auto ret = this->send(&info);
        if(ret) {
          std::cout << "ZeroMQ sending failed!!!" << std::endl;
        }
      }
    }

    DoocsProcessScalar(EqFct* eqFct, std::string doocsPropertyName,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
    : DOOCS_T(eqFct, doocsPropertyName.c_str()), _processScalar(processScalar), _doocsUpdater(updater), _eqFct(eqFct) {
      if(processScalar->isReadable()) {
        updater.addVariable(ChimeraTK::ScalarRegisterAccessor<T>(processScalar), eqFct,
            std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this, processScalar->getId()));
        _consistencyGroup.add(processScalar);

        // put variable into error state, until a valid value has been received
        if(!_processScalar->isWriteable()) {
          this->d_error(stale_data);
        }
      }
    }

    DoocsProcessScalar(std::string doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
    : DOOCS_T(doocsPropertyName.c_str(), eqFct), _processScalar(processScalar), _doocsUpdater(updater), _eqFct(eqFct) {
      if(processScalar->isReadable()) {
        updater.addVariable(ChimeraTK::ScalarRegisterAccessor<T>(processScalar), eqFct,
            std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this, processScalar->getId()));
        _consistencyGroup.add(processScalar);

        // put variable into error state, until a valid value has been received
        if(!_processScalar->isWriteable()) {
          this->d_error(stale_data);
        }
      }
    }

    /**
     * Override the Doocs set method which is triggered by the RPC calls.
     */
    void set(EqAdr* adr, EqData* data1, EqData* data2, EqFct* eqfct) override {
      // only assign the value if the variable is writeable
      // Otherwise the content displayed by Doocs and the value in the application
      // are inconsistent
      if(_processScalar->isWriteable()) {
        DOOCS_T::set(adr, data1, data2, eqfct);
        // let the DOOCS_T set function do all the dirty work and use the
        // get_value function afterwards to get the already assigned value
        _processScalar->accessData(0) = this->value();
        _processScalar->write();
      }
      else {
        throw ChimeraTK::logic_error("Trying to write to a non-writable variable");
      }
      // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
      if(_publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
        auto timestamp = _processScalar->getVersionNumber().getTime();
        auto seconds = std::chrono::system_clock::to_time_t(timestamp);
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
            timestamp - std::chrono::system_clock::from_time_t(seconds))
                                .count();
        dmsg_info info;
        memset(&info, 0, sizeof(info));
        info.sec = seconds;
        info.usec = microseconds;
        if(_macroPulseNumberSource != nullptr) {
          this->set_mpnum(_macroPulseNumberSource->accessData(0));
          info.ident = _macroPulseNumberSource->accessData(0);
        }
        else {
          info.ident = 0;
        }
        auto ret = this->send(&info);
        if(ret) {
          std::cout << "ZeroMQ sending failed!!!" << std::endl;
        }
      }
    }

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init(void) override {
      DOOCS_T::auto_init();
      // send the current value to the device
      if(_processScalar->isWriteable()) {
        _processScalar->accessData(0) = DOOCS_T::value();
        _processScalar->write();
        // set DOOCS time stamp, workaround for DOOCS bug (get() always gives current time stamp if no timestamp is set,
        // which breaks consistency check in ZeroMQ subscriptions after the 4 minutes timeout)
        DOOCS_T::set_stamp();
      }
    }

    void publishZeroMQ() { _publishZMQ = true; }

    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
        _macroPulseNumberSource = macroPulseNumberSource;
        if(_consistencyGroup.getMatchingMode() != DataConsistencyGroup::MatchingMode::none) {
          _consistencyGroup.add(macroPulseNumberSource);
          _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct,
              std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this, macroPulseNumberSource->getId()));
        }
        else {
          // We don't need to match up anything with it when it changes, but we have to register this at least once
          // so the macropulse number will be included in the readAnyGroup in the updater if
          // <data_matching> is none everywhere
          _doocsUpdater.addVariable(
              ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct, []() {});
        }
    }

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> _processScalar;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
    DataConsistencyGroup _consistencyGroup;
    DoocsUpdater& _doocsUpdater; // store the reference to the updater. We need it when adding the macro pulse number
    EqFct* _eqFct;               // We need it when adding the macro pulse number
    bool _publishZMQ{false};
    bool _doocsSuccessfullyUpdated{true}; // to detect data losses

    // counter used to reduce amount of data loss warnings printed at console
    size_t _nDataLossWarnings{0};
  };

} // namespace ChimeraTK

#endif // __DOOCS_PROCESS_SCALAR_H__
