#pragma once

#include "DoocsUpdater.h"
#include "MappedImage.h"
#include "PropertyBase.h"
#include <eq_fct.h>

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

namespace ChimeraTK {

  /*
   * First concept for image in ApplicatonCore and mapping to DOOCS.
   *
   * - Use 1D register accessor with fixed size (upper limit) to store image,
   *   consisting of header and data. This ensures data consistency.
   * - keep some flexibility with regard of the UserType of the array.
   * - provide mapping to some DOOCS formats
   *
   */

  /// implements DOOCS-specific mapping for MappedImage
  struct MappedDoocsImg : public MappedImage {
    using MappedImage::MappedImage;

    /// Overwrites headerOut and returns pointer to internal data, without header
    /// The DOOCS image format is selected based on previously set header info, i.e. channels and bpp.
    unsigned char* asDoocsImg(IMH* headerOut) {
      assert((_header->options & ImgOpt_RowMajor) && "conversion to DOOCS image only possible for row-major ordering");

      if(_header->channels == 1 && _header->bpp == 1) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_GRAY;
      }
      else if(_header->channels == 1 && _header->bpp == 2) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_GRAY;
      }
      else if(_header->channels == 3 && _header->bpp == 3) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_RGB;
      }
      else if(_header->channels == 4 && _header->bpp == 4) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_RGBA;
      }
      else {
        assert(false && "image format not supported!");
      }

      headerOut->width = _header->width;
      headerOut->height = _header->height;
      headerOut->bpp = _header->bpp;
      headerOut->image_flags = TTF2_IMAGE_LOSSLESS;
      headerOut->source_format = headerOut->image_format;
      headerOut->frame = _header->frame;
      headerOut->aoi_width = _header->width; // TODO look up meaning
      headerOut->aoi_height = _header->height;
      headerOut->length = headerOut->aoi_width * headerOut->aoi_height * headerOut->bpp;
      headerOut->x_start = 0;
      headerOut->y_start = 0;
      headerOut->ebitpp = _header->ebitpp;
      headerOut->hbin = 1;   // TODO ask for meaning
      headerOut->vbin = 1;
      headerOut->event = _header->event; // TODO ask meaning difference event vs. frame
      headerOut->scale_x = 0;
      headerOut->scale_y = 0;
      headerOut->image_rotation = 0;
      return _data + sizeof(ImgHeader);
    }
  };

  void demo_imageProc() {
    std::cout << "demo_imageProc" << std::endl;

    // std::vector<uint8_t> buf(10000); // to be replaced later by buffer of 1D register accessor
    // MappedImage A0(buf);
    MappedImage A0;

    A0.setShape(10, 30, 1, 2);
    auto Av = A0.interpretedView<short>();
    Av(0, 0) = 8;
    Av(1, 0) = 7;
    Av(0, 2) = 4;
    Av(0, 3) = 5;
    Av(0, 4) = 6;
    float val = Av(0, 2);
    std::cout << " pixel val = " << val << std::endl;

    MappedDoocsImg A(A0.data(), A0.dataLen(), false);
    IMH headerOut;
    unsigned char* imgData = A.asDoocsImg(&headerOut);
    std::cout << " img data: " << val << std::endl;
    for(int i = 0; i < 200; i++) std::cout << (int)imgData[i] << ", ";
    std::cout << std::endl;
  }

  class DoocsImage : public D_imagec, public PropertyBase {
   public:
    DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> const& processArray, DoocsUpdater& updater);

    IMH _imh;
    IMH* getIMH() { return &_imh; }

   protected:
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> _processArray;
  };

  /*******************************************************************************/

  inline DoocsImage::DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> const& processArray, DoocsUpdater& updater)
  : // D_image(doocsPropertyName, processArray->getNumberOfSamples(), eqFct),
    D_imagec(doocsPropertyName, eqFct), PropertyBase(doocsPropertyName, updater), _processArray(processArray) {
    if(_processArray->isWriteable()) {
      std::cout << "WARNING: writable images not supported by DoocsImage" << std::endl;
    }
    setupOutputVar(processArray);
  }

  inline void DoocsImage::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    D_imagec* dfct = this;
    // D_image* dfct = this;
    //  Note: we already own the location lock by specification of the
    //  DoocsUpdater

    MappedDoocsImg img(_processArray->accessChannel(0).data(), _processArray->accessChannel(0).size(), false);
    auto* dataPtr = img.asDoocsImg(&_imh);

    if(_processArray->dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    if(_macroPulseNumberSource) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
      _imh.event = _macroPulseNumberSource->accessData(0);
    }
    else {
      // TODO discuss this case. event might be provided in image header already.
      //_imh.event = _imh.frame;
    }

    // this copies header and data contents to DOOCS-internal buffer
    dfct->set_value(&_imh, dataPtr);
    // for D_image:
    // dfct->set_img_header(&_imh);
    // dfct->fill_image('4');
    // memcpy(dfct->get_image(), dataPtr, _imh.length);

    auto ts = timestamp.get_seconds_and_microseconds_since_epoch();
    // TODO - check wheter we need this in addition to usual dfct calls
    dfct->set_img_time(ts.seconds, ts.microseconds);
    // dfct->set_img_status();

    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
