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
   *     The user can choose to have either memory-efficient storage or easy
   *     computations with floats.
   * - provide mapping to some DOOCS formats
   *
   */

  //  here comes DOOCS - independent classes.
  // TODO move to ControlsystemAdapter?

  enum class Encoding {
    // this actually is a lot.
    // combinations of :
    // (row/header first)
    // byte order
    // number of bytes per value
    // float/int
    // number of channels
    // or even things that don't match here: compressed, ...
    // so-> just a reasonable short subset
    Gray_1b,
    Gray_2b,
    RGB_3b,
    RGBA_4b
  };

  const int ImgOpt_RowMajor = 1;
  const int ImgOpt_ColMajor = 0;

  /**
   * Provides the interface to an image that is mapped onto a 1D array of ValType
   * data members are only used for optimizating, everything is really stored in the array
   */
  template<typename ValType, int OPTIONS = ImgOpt_RowMajor>
  struct MappedImage {
    enum HeaderPositions {
      // defines header layout in our array
      // TODO we might want to switch to a more flexible approach based on encoding / decoding values as byte string
      RES,
      RES2,
      WIDTH,
      WIDTH2,
      HEIGHT,
      HEIGHT2,
      CHANNELS,
      CHANNELS2,
      END
    };

    MappedImage(std::vector<ValType>& data) {
      _vec = &data;
      // read shape from header, assuming minimal value range of ValType
      _width = decValue(HeaderPositions::WIDTH);
      _height = decValue(HeaderPositions::HEIGHT);
      _channels = decValue(HeaderPositions::CHANNELS);
    }
    void setShape(unsigned width, unsigned height, unsigned channels = 1) {
      assert(width * height * channels + _headerLen <= _vec->size());
      _width = width;
      _height = height;
      _channels = channels;
      // set shape in header
      encValue(HeaderPositions::WIDTH, _width);
      encValue(HeaderPositions::HEIGHT, _height);
      encValue(HeaderPositions::CHANNELS, _channels);
    }
    std::vector<ValType>* _vec;

    void encValue(unsigned pos, int value) {
      // simplest approach
      (*_vec)[pos] = value % 128;
      (*_vec)[pos + 1] = value / 128;
    }
    int decValue(unsigned pos) { return (*_vec)[pos] + (*_vec)[pos + 1] * 128; }

    template<typename TargetType>
    void doEncode(std::vector<unsigned char>& destBuf, unsigned bpp) {
      const unsigned outHeaderLen = sizeof(IMH);
      unsigned npixels = _width * _height;
      unsigned nvalues = npixels * _channels;
      destBuf.resize(npixels * bpp + outHeaderLen);
      auto pbase = destBuf.data() + outHeaderLen;
      if constexpr(std::is_same<ValType, TargetType>::value) {
        // use optimization, since no casts needed
        memcpy(pbase, _vec->data(), npixels * bpp);
      }
      else {
        for(unsigned i = 0; i < nvalues; i++) {
          *reinterpret_cast<TargetType*>(pbase + i) = (TargetType)(*_vec)[i];
        }
      }
    }

    ValType& operator()(unsigned x, unsigned y, unsigned c = 0) {
      // this is the only place where row-major / column-major storage is decided
      if constexpr(OPTIONS & ImgOpt_RowMajor) {
        return (*_vec)[_headerLen + (y * _width + x) * _channels + c];
      }
      else {
        return (*_vec)[_headerLen + (y + x * _height) * _channels + c];
      }
    }
    unsigned _headerLen = HeaderPositions::END;
    unsigned _width = 1;
    unsigned _height = 1;
    unsigned _channels = 1;
  };

  template<typename ValType, int OPTIONS = ImgOpt_RowMajor>
  struct MappedDoocsImg : public MappedImage<ValType, OPTIONS> {
    using MappedImage<ValType, OPTIONS>::MappedImage;

    void encodeAsDoocsImg(Encoding selectedEnc, std::vector<unsigned char>& destBuf) {
      assert((OPTIONS & ImgOpt_RowMajor) && "conversion to DOOCS image only implemented for ROW_MAJOR");

      int image_format;
      unsigned bpp;
      switch(selectedEnc) {
        case Encoding::Gray_1b:
          bpp = 1;
          assert(this->_channels == 1);
          this->template doEncode<uint8_t>(destBuf, bpp);
          image_format = TTF2_IMAGE_FORMAT_GRAY;
          break;
        case Encoding::Gray_2b:
          bpp = 2;
          assert(this->_channels == 1);
          this->template doEncode<uint16_t>(destBuf, bpp);
          image_format = TTF2_IMAGE_FORMAT_GRAY;
          break;
        case Encoding::RGB_3b:
          bpp = 3;
          assert(this->_channels == 3);
          this->template doEncode<uint8_t>(destBuf, bpp);
          image_format = TTF2_IMAGE_FORMAT_RGB;
          break;
        case Encoding::RGBA_4b:
          bpp = 4;
          assert(this->_channels == 4);
          this->template doEncode<uint8_t>(destBuf, bpp);
          image_format = TTF2_IMAGE_FORMAT_RGBA;
          break;
      }

      // header destination
      IMH* imh = (IMH*)destBuf.data();
      imh->image_format = image_format;
      imh->width = this->_width;
      imh->height = this->_height;
      imh->bpp = bpp;
      imh->image_format = TTF2_IMAGE_FORMAT_GRAY;
      imh->image_flags = TTF2_IMAGE_LOSSLESS;
      imh->source_format = image_format;
      imh->frame = 1;
      imh->aoi_width = imh->width;
      imh->aoi_height = imh->height;
      imh->length = imh->aoi_width * imh->aoi_height * imh->bpp;
    }
  };

  void demo_imageProc() {
    std::cout << "demo_imageProc" << std::endl;

    std::vector<uint8_t> buf(10000); // to be replaced later by buffer of 1D register accessor

    MappedImage<uint8_t, ImgOpt_RowMajor> A0(buf);
    MappedDoocsImg<uint8_t> A(buf);
    A.setShape(10, 30);
    A(0, 2) = 4;
    A(0, 3) = 5;
    A(0, 4) = 6;
    A(1, 0) = 7;
    float val = A(0, 2);

    std::cout << " pixel val = " << val << std::endl;
    std::vector<u_char> destbuf;
    A.encodeAsDoocsImg(Encoding::Gray_1b, destbuf);
    for(int i = 0; i < 200; i++) std::cout << (void*)destbuf[i] << ", ";
    std::cout << std::endl;
  }

  // TODO check - does it make sense to template this on base data type?

  class DoocsImage : public D_imagec, public PropertyBase {
   public:
    DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<u_char>> const& processArray, DoocsUpdater& updater);

    std::vector<uint8_t> _imageData;
    IMH* getIMH() { return (IMH*)_imageData.data(); }

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

    MappedDoocsImg<uint8_t> img(_processArray->accessChannel(0));
    // TODO - how choose right encoding?

    img.encodeAsDoocsImg(Encoding::Gray_1b, _imageData);
    IMH* imh = getIMH();
    u_char* dataPtr = _imageData.data() + sizeof(IMH);

    if(_processArray->dataValidity() != ChimeraTK::DataValidity::ok) {
      this->d_error(stale_data);
    }
    else {
      this->d_error(no_error);
    }

    doocs::Timestamp timestamp = correctDoocsTimestamp();

    if(_macroPulseNumberSource) {
      this->set_mpnum(_macroPulseNumberSource->accessData(0));
      imh->event = _macroPulseNumberSource->accessData(0);
    }
    else {
      // TODO discuss this case
      imh->event = imh->frame;
    }
    // TODO
    imh->bpp = 2;     // hold->getBytesPerPix();
    imh->ebitpp = 14; // hold->getEffBitsPerPix();

    // TODO get from meta data? or automatically count up in here?
    imh->frame = 1;
    dfct->set_value(imh, dataPtr);
    // TODO check validity
    // dataPtr / imh pointer locations could change between function calls
    // since encodeAsDoocsImg() resizes output vector
    // (but normally that won't happen)
    // I guess that's ok as long as we do these changes only while having location lock

    // TODO - check wheter we need this in addition to usual dfct calls
    // dfct->set_img_time()
    // dfct->set_img_status();
    // dfct->set_descr_value() // good candidate for config var

    sendZMQ(timestamp);
  }

} // namespace ChimeraTK
