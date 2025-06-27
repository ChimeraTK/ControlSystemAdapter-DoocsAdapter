// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "DoocsImage.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

namespace ChimeraTK {

  unsigned char* MappedDoocsImg::asDoocsImg(IMH* headerOut) {
    auto* h = header();
    switch(h->image_format) {
      case ImgFormat::Unset:
        return nullptr;
      case ImgFormat::Gray8:
      case ImgFormat::Gray16:
        headerOut->image_format = TTF2_IMAGE_FORMAT_GRAY;
        break;
      case ImgFormat::RGB24:
        headerOut->image_format = TTF2_IMAGE_FORMAT_RGB;
        break;
      case ImgFormat::RGBA32:
        headerOut->image_format = TTF2_IMAGE_FORMAT_RGBA;
        break;
      default:
        assert(false && "image format not supported!");
    }
    if(!((unsigned)h->options & (unsigned)ImgOptions::RowMajor)) {
      throw logic_error("conversion to DOOCS image only possible for row-major ordering");
    }

    headerOut->width = h->width;
    headerOut->height = h->height;
    headerOut->bpp = h->bytesPerPixel;
    headerOut->image_flags = TTF2_IMAGE_LOSSLESS | TTF2_IMAGE_LITTLE_ENDIAN_BYTE_ORDER;
    headerOut->source_format = headerOut->image_format;
    headerOut->frame = h->frame;
    // area of interest: in our case, always the full image
    headerOut->aoi_width = h->width;
    headerOut->aoi_height = h->height;
    headerOut->length = headerOut->aoi_width * headerOut->aoi_height * headerOut->bpp;
    headerOut->x_start = h->x_start;
    headerOut->y_start = h->y_start;
    headerOut->ebitpp = h->effBitsPerPixel;
    headerOut->hbin = 1;
    headerOut->vbin = 1;
    headerOut->event = 0; // this will be overwritten if macropulse source was set
    // unused values are set to -1
    headerOut->scale_x = h->scale_x;
    headerOut->scale_y = h->scale_y;
    headerOut->image_rotation = 0;
    headerOut->fspare2 = -1;
    headerOut->fspare3 = -1;
    headerOut->fspare4 = -1;
    headerOut->ispare2 = -1;
    headerOut->ispare3 = -1;
    headerOut->ispare4 = -1;
    return data() + sizeof(ImgHeader);
  }

  /************************* DoocsImage ************************************************************************/

  DoocsImage::DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> const& processArray, DoocsUpdater& updater,
      DataConsistencyGroup::MatchingMode matchingMode)
  : D_imagec(doocsPropertyName, eqFct), PropertyBase(doocsPropertyName, updater, matchingMode),
    _processArray(processArray) {
    if(_processArray.isWriteable()) {
      // It could only be writable if the application implements it as an output with back-channel.
      // Then consider this application to have a logical bug.
      throw logic_error("writable images not supported by DoocsImage");
    }
    setupOutputVar(_processArray);
  }

  void DoocsImage::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    D_imagec* dfct = this;
    //  Note: we already own the location lock by specification of the DoocsUpdater

    MappedDoocsImg img(_processArray, MappedDoocsImg::InitData::No);
    auto* dataPtr = img.asDoocsImg(&imh);
    if(!dataPtr) {
      throw logic_error("data provided to DoocsImage._processArray was not recognized as image");
    }

    if(_processArray.dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    if(_macroPulseNumberSource.isInitialised()) {
      this->set_mpnum(_macroPulseNumberSource);
      imh.event = _macroPulseNumberSource;
    }

    // this copies header and data contents to DOOCS-internal buffer
    dfct->set_value(&imh, dataPtr);

    auto ts = timestamp.get_seconds_and_microseconds_since_epoch();
    // this is needed in addition to usual dfct call, in order to set time in img meta data
    // note, some meta data is not correctly shown by DOOCS rpc interface but is in ZMQ.
    dfct->set_img_time(ts.seconds, ts.microseconds);
    // dfct->set_img_status();

    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
