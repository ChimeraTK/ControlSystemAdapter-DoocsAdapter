#pragma once

#include "DoocsUpdater.h"
#include "PropertyBase.h"
#include <eq_fct.h>

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

namespace ChimeraTK {

  // TODO check - does it make sense to template this on base data type?

  class DoocsImage : public D_imagec, public PropertyBase {
   public:
    DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<u_char>> const& processArray, DoocsUpdater& updater);

    IMH* getIMH() { return &_imh; }

   protected:
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> _processArray;
    IMH _imh;
    bool dbg = true;
  };

  /*******************************************************************************/

  inline DoocsImage::DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> const& processArray, DoocsUpdater& updater)
  : D_imagec(doocsPropertyName, eqFct), PropertyBase(doocsPropertyName, updater), _processArray(processArray) {
    if(_processArray->isWriteable()) {
      std::cout << "WARNING: writable images not supported by DoocsImage" << std::endl;
    }
    setupOutputVar(processArray);

    _imh.width = 256;
    _imh.height = 100;
    _imh.bpp = 2;     // hold->getBytesPerPix();
    _imh.ebitpp = 14; // hold->getEffBitsPerPix();
    _imh.aoi_width = _imh.width;
    _imh.aoi_height = _imh.height;
    _imh.length = _imh.aoi_width * _imh.aoi_height * _imh.bpp;
    _imh.source_format = TTF2_IMAGE_FORMAT_GRAY;
    _imh.image_format = TTF2_IMAGE_FORMAT_GRAY;
    _imh.image_flags = TTF2_IMAGE_LOSSLESS;
  }

  void DoocsImage::DoocsImage::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    D_imagec* dfct = this;

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

    assert(_imh.length <= (int)processVector.size());
    dfct->set_value(&_imh, dataPtr);

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    if(_macroPulseNumberSource) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
    }
    // TODO - check wheter we need this in addition to usual dfct calls
    // dfct->set_img_time()
    // dfct->set_img_status();
    // dfct->set_descr_value()

    _imh.frame = 1; // it->frames_.value();
    _imh.event = 2; // TODO macropulse nr.?

    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
