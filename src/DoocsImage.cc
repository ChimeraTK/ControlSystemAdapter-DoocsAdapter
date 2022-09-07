#include "DoocsImage.h"

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

namespace ChimeraTK {

  unsigned char* MappedDoocsImg::asDoocsImg(IMH* headerOut) {
    switch(_header->image_format) {
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
    assert(((unsigned)_header->options & (unsigned)ImgOptions::RowMajor) &&
        "conversion to DOOCS image only possible for row-major ordering");

    headerOut->width = _header->width;
    headerOut->height = _header->height;
    headerOut->bpp = _header->bpp;
    headerOut->image_flags = TTF2_IMAGE_LOSSLESS;
    headerOut->source_format = headerOut->image_format;
    headerOut->frame = _header->frame;
    // area of interest: in our case, always the full image
    headerOut->aoi_width = _header->width;
    headerOut->aoi_height = _header->height;
    headerOut->length = headerOut->aoi_width * headerOut->aoi_height * headerOut->bpp;
    headerOut->x_start = _header->x_start;
    headerOut->y_start = _header->y_start;
    headerOut->ebitpp = _header->ebitpp;
    headerOut->hbin = 1;
    headerOut->vbin = 1;
    headerOut->event = 0; // this will be overwritten if macropulse source was set
    // unused values are set to -1
    headerOut->scale_x = _header->scale_x;
    headerOut->scale_y = _header->scale_y;
    headerOut->image_rotation = 0;
    headerOut->fspare2 = -1;
    headerOut->fspare3 = -1;
    headerOut->fspare4 = -1;
    headerOut->ispare2 = -1;
    headerOut->ispare3 = -1;
    headerOut->ispare4 = -1;
    return _data + sizeof(ImgHeader);
  }

  /************************* DoocsImage ************************************************************************/

  DoocsImage::DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
      boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> const& processArray, DoocsUpdater& updater)
  : D_imagec(doocsPropertyName, eqFct), PropertyBase(doocsPropertyName, updater), _processArray(processArray) {
    if(_processArray->isWriteable()) {
      std::cout << "WARNING: writable images not supported by DoocsImage" << std::endl;
    }
    setupOutputVar(processArray);
  }

  void DoocsImage::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    D_imagec* dfct = this;
    //  Note: we already own the location lock by specification of the DoocsUpdater

    MappedDoocsImg img(
        _processArray->accessChannel(0).data(), _processArray->accessChannel(0).size(), MappedDoocsImg::InitData::No);
    auto* dataPtr = img.asDoocsImg(&_imh);
    if(!dataPtr) {
      throw logic_error("data provided to DoocsImage._processArray was not recognized as image");
    }

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

    // this copies header and data contents to DOOCS-internal buffer
    dfct->set_value(&_imh, dataPtr);

    auto ts = timestamp.get_seconds_and_microseconds_since_epoch();
    // this is needed in addition to usual dfct call, in order to set time in img meta data
    // note, some meta data is not correctly shown by DOOCS rpc interface but is in ZMQ.
    dfct->set_img_time(ts.seconds, ts.microseconds);
    // dfct->set_img_status();

    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
