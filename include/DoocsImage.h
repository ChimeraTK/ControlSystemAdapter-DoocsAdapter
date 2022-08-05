#pragma once

#include "DoocsUpdater.h"
#include "PropertyBase.h"
#include <eq_fct.h>

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

#include <assert.h>
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

  //  here comes DOOCS - independent classes.
  // TODO move to ControlsystemAdapter?

  // Image header defined like doocs::IMH, except we use defined-size types
  struct ImgHeader {
    uint32_t width;
    uint32_t height;
    uint32_t aoi_width;
    uint32_t aoi_height;
    uint32_t x_start;
    uint32_t y_start;
    uint32_t bpp;
    uint32_t ebitpp;
    uint32_t hbin;
    uint32_t vbin;
    uint32_t source_format;
    uint32_t image_format;
    uint32_t frame;
    uint32_t event;
    float scale_x;
    float scale_y;
    float image_rotation;
    float fspare2;
    float fspare3;
    float fspare4;
    uint32_t image_flags;
    uint32_t channels;
    uint32_t options;
    uint32_t ispare4;
    uint32_t length;
  };
  // values for the options bit field
  const int ImgOpt_RowMajor = 1;

  template<typename ValType, unsigned OPTIONS>
  class ImgView {
    friend class MappedImage;

   public:
    ValType& operator()(unsigned x, unsigned y, unsigned c = 0) {
      // this is the only place where row-major / column-major storage is decided
      if constexpr(OPTIONS & ImgOpt_RowMajor) {
        return _vec[(y * _h->width + x) * _h->channels + c];
      }
      else {
        return _vec[(y + x * _h->height) * _h->channels + c];
      }
    }

   protected:
    ImgHeader* _h;
    ValType* _vec;
  };

  /**
   * Provides the interface to an image that is mapped onto a 1D array of ValType
   */
  class MappedImage {
   public:
    /// call with initData=false if data already contains valid image.
    explicit MappedImage(std::vector<unsigned char>& data, bool initData = true) {
      _buf = &data;
      _imh = reinterpret_cast<ImgHeader*>(_buf->data());
      if(initData) {
        memset(data.data(), 0, data.size());
        _imh->options = ImgOpt_RowMajor;
      }
    }
    /// provided for convenience, if MappedImage should allocate the data vector
    MappedImage() {
      _allocate = true;
      _buf = new std::vector<unsigned char>(sizeof(ImgHeader));
      _imh = reinterpret_cast<ImgHeader*>(_buf->data());
      _imh->options = ImgOpt_RowMajor;
    }
    ~MappedImage() {
      if(_allocate) delete _buf;
    }
    std::vector<unsigned char>& data() { return *_buf; }

    /// needs to be called after construction. corrupts all data except header.
    void setShape(unsigned width, unsigned height, unsigned channels = 1, unsigned bpp = 1) {
      unsigned headerLen = sizeof(ImgHeader);
      if(!_allocate) {
        assert(width * height * bpp + headerLen <= _buf->size());
      }
      else {
        _buf->resize(width * height * bpp + headerLen);
        _imh = reinterpret_cast<ImgHeader*>(_buf->data());
      }
      _imh->width = width;
      _imh->height = height;
      _imh->channels = channels;
      _imh->bpp = bpp;
    }

    /// returns an ImgView object which can be used like a matrix. The ImgView becomes invalid at next setShape call.
    template<typename UserType, int OPTIONS = ImgOpt_RowMajor>
    ImgView<UserType, OPTIONS> interpretedView() {
      assert(_imh->channels > 0 && "call setShape() before interpretedView()!");
      assert(
          _imh->bpp == _imh->channels * sizeof(UserType) && "choose correct bpp and channels value before conversion!");
      ImgView<UserType, OPTIONS> ret;
      ret._h = reinterpret_cast<ImgHeader*>(_buf->data());
      ret._vec = reinterpret_cast<UserType*>(_buf->data() + sizeof(ImgHeader));
      return ret;
    }

   protected:
    bool _allocate = false;
    ImgHeader* _imh;
    std::vector<unsigned char>* _buf; // pointer to data for header and image
  };

  struct MappedDoocsImg : public MappedImage {
    using MappedImage::MappedImage;

    /// Overwrites headerOut and returns pointer to internal data, without header
    /// The DOOCS image format is selected based on previously set header info, i.e. channels and bpp.
    unsigned char* asDoocsImg(IMH* headerOut) {
      assert((_imh->options & ImgOpt_RowMajor) && "conversion to DOOCS image only possible for row-major ordering");

      if(_imh->channels == 1 && _imh->bpp == 1) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_GRAY;
      }
      else if(_imh->channels == 1 && _imh->bpp == 2) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_GRAY;
      }
      else if(_imh->channels == 3 && _imh->bpp == 3) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_RGB;
      }
      else if(_imh->channels == 4 && _imh->bpp == 4) {
        headerOut->image_format = TTF2_IMAGE_FORMAT_RGBA;
      }
      else {
        assert(false && "image format not supported!");
      }

      headerOut->width = _imh->width;
      headerOut->height = _imh->height;
      headerOut->bpp = _imh->bpp;
      headerOut->image_flags = TTF2_IMAGE_LOSSLESS;
      headerOut->source_format = headerOut->image_format;
      headerOut->frame = 1;
      headerOut->aoi_width = _imh->width; // TODO look up meaning
      headerOut->aoi_height = _imh->height;
      headerOut->length = headerOut->aoi_width * headerOut->aoi_height * headerOut->bpp;
      headerOut->x_start = 0;
      headerOut->y_start = 0;
      headerOut->ebitpp = 8; // TODO
      headerOut->hbin = 1;   // TODO ask for meaning
      headerOut->vbin = 1;
      headerOut->event = 1; // TODO
      headerOut->scale_x = 0;
      headerOut->scale_y = 0;
      headerOut->image_rotation = 0;
      return _buf->data() + sizeof(ImgHeader);
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

    MappedDoocsImg A(A0.data(), false);
    IMH headerOut;
    unsigned char* imgData = A.asDoocsImg(&headerOut);
    std::cout << " img data: " << val << std::endl;
    for(int i = 0; i < 200; i++) std::cout << (void*)imgData[i] << ", ";
    std::cout << std::endl;
  }

  class DoocsImage : public D_imagec, public PropertyBase {
   public:
    DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> const& processArray, DoocsUpdater& updater);

    std::vector<uint8_t> _imageData;
    IMH _imh;
    IMH* getIMH() { return &_imh; }

   protected:
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> _processArray;
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
  }

  inline void DoocsImage::updateDoocsBuffer(const TransferElementID& transferElementId) {
    if(!updateConsistency(transferElementId)) {
      return;
    }

    D_imagec* dfct = this;
    // Note: we already own the location lock by specification of the
    // DoocsUpdater

    MappedDoocsImg img(_processArray->accessChannel(0), false);
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
      // TODO discuss this case
      _imh.event = _imh.frame;
    }
    // TODO in app
    // imh->bpp = 2;     // hold->getBytesPerPix();
    // imh->ebitpp = 14; // hold->getEffBitsPerPix();
    // imh->frame = 1;

    // dataPtr location could change between function calls
    // but I think that's ok as long as we do these changes only while having location lock
    dfct->set_value(&_imh, dataPtr);

    // TODO - check wheter we need this in addition to usual dfct calls
    // dfct->set_img_time()
    // dfct->set_img_status();
    // dfct->set_descr_value() // good candidate for config var

    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
