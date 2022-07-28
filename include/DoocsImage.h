#pragma once

#include <boost/noncopyable.hpp>

#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h> // needed for the macro pulse number
#include <ChimeraTK/DataConsistencyGroup.h>

#include "DoocsUpdater.h"
#include "DoocsAdapter.h"
#include "DoocsProcessArray.h"

#include <eq_fct.h>

namespace ChimeraTK {

  typedef DoocsProcessArray<D_imagec, u_char> DoocsImageBase;

  class DoocsImage : public DoocsImageBase {
    IMH _imh;
    bool dbg = true;

   public:
    DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<u_char>> const& processArray, DoocsUpdater& updater);

    void publishAsDoocsImg();
    IMH* getIMH() { return &_imh; }
  };

  // TODO - let't try template specialization approach in order to
  // minimize changes on DoocsImageBase / DoocsProcessArray.
  // We need:
  // constructor, updateDoocsBuffer, ?
  template<>
  DoocsProcessArray<D_imagec, u_char>::DoocsProcessArray(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<u_char>> const& processArray, DoocsUpdater& updater)
  // TODO D_imagec needs different constructor here
  // TODO check handling of array dim change - is it possible at all?
  : D_imagec(doocsPropertyName.c_str(), eqFct), _processArray(processArray), _doocsUpdater(updater), _eqFct(eqFct) {
    if(processArray->isReadable()) {
      updater.addVariable(ChimeraTK::OneDRegisterAccessor<u_char>(processArray), eqFct,
          std::bind(&DoocsProcessArray<D_imagec, u_char>::updateDoocsBuffer, this, processArray->getId()));
      _consistencyGroup.add(processArray);
    }
    // put variable into error state, until a valid value has been received
    if(!_processArray->isWriteable()) {
      this->d_error(stale_data);
    }
  }

  template<>
  void DoocsProcessArray<D_imagec, u_char>::updateDoocsBuffer(const TransferElementID& transferElementId) {
    // FIXME: A first  implementation is checking the data consistency here. Later this should be
    // before calling this function because calling this function through a function pointer is
    // comparatively expensive.
    // Only check the consistency group if there is a macro pulse number associated.
    // Do not check if update is coming from another DOOCS property mapped to the same variable (ID invalid), since
    // the check would never pass. Such variables cannot use exact data matching anyway, since the update is triggered
    // from the DOOCS write to the other property.
    if(transferElementId.isValid() && _macroPulseNumberSource && !_consistencyGroup.update(transferElementId)) {
      // data is not consistent (yet). Don't update the Doocs buffer.
      // check if this will now throw away data and generate a warning
      if(transferElementId == _processArray->getId()) {
        if(!_doocsSuccessfullyUpdated) {
          ++_nDataLossWarnings;
          if(DoocsAdapter::checkPrintDataLossWarning(_nDataLossWarnings)) {
            std::cout << "WARNING: Data loss in array property " << _eqFct->name() << "/" << this->basename()
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
    auto& processVector = _processArray->accessChannel(0);

    auto dataPtr = processVector.data();

    if(_processArray->dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    IMH* imh = static_cast<DoocsImage*>(this)->getIMH();
    assert(imh->length <= (int)processVector.size());
    this->set_value(imh, dataPtr);

    modified = true;

    // Convert time stamp from version number to DOOCS timestamp
    doocs::Timestamp timestamp(_processArray->getVersionNumber().getTime());

    // Make sure we never send out two absolute identical time stamps. If we would do so, the "watchdog" which
    // corrects inconsistencies in ZeroMQ subscriptions between sender and subcriber cannot detect the inconsistency.
    if(this->get_timestamp() == timestamp) {
      timestamp += std::chrono::microseconds(1);
    }

    this->set_timestamp(timestamp);
    if(_macroPulseNumberSource) this->set_mpnum(_macroPulseNumberSource->accessData(0));

    // send data via ZeroMQ if enabled and if DOOCS initialisation is complete
    if(publishZMQ && ChimeraTK::DoocsAdapter::isInitialised) {
      dmsg_info info;
      memset(&info, 0, sizeof(info));
      auto sinceEpoch = timestamp.get_seconds_and_microseconds_since_epoch();
      info.sec = sinceEpoch.seconds;
      info.usec = sinceEpoch.microseconds;
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

  inline DoocsImage::DoocsImage(EqFct* eqFct, const std::string& doocsPropertyName,
      const boost::shared_ptr<ChimeraTK::NDRegisterAccessor<u_char>>& processArray, DoocsUpdater& updater)
  : DoocsImageBase(eqFct, doocsPropertyName, processArray, updater) {
    _imh.width = 256;
    _imh.height = 100;
    _imh.bpp = 2;     //hold->getBytesPerPix();
    _imh.ebitpp = 14; //hold->getEffBitsPerPix();
    _imh.aoi_width = _imh.width;
    _imh.aoi_height = _imh.height;
    _imh.length = _imh.aoi_width * _imh.aoi_height * _imh.bpp;
    _imh.source_format = TTF2_IMAGE_FORMAT_GRAY;
    _imh.image_format = TTF2_IMAGE_FORMAT_GRAY;
    _imh.image_flags = TTF2_IMAGE_LOSSLESS;
  }

  inline void DoocsImage::publishAsDoocsImg() {
    struct timeval tm;

    int gen_event_nm = 0;

    _imh.frame = 1; //it->frames_.value();
    _imh.event = 2; // TODO macropulse nr.?

    u_char* buf;
    //D_imagec image_("IMAGE image one shot", this->getEqFct());
    D_imagec& image_ = *this;
    //image_.set_mode(DMSG_EN);

    //TODO check whether we need this above
    image_.set_img_time(tm.tv_sec, tm.tv_usec);
    image_.set_img_status(_imh.event);
    //image_.set_value(&_imh, buf);
    //it->data_ready = 1;

    //    dmsg_info_t db;
    //    db.sec = tm.tv_sec;
    //    db.usec = tm.tv_usec;
    //    db.ident = gen_event_nm;
    //    db.stat = 0;
    //    image_.send(&db);

    //      if(dbg) {
    //        char msg[256];
    //        snprintf(msg, sizeof(msg), "Image sent with %ld:%ld_%d", tm.tv_sec, tm.tv_usec, gen_event_nm);
    //        printtostderr(it->name_.value(), msg);
    //      }
  }

} // namespace ChimeraTK
