#pragma once

// TODO
// discuss where this file belongs. It only depends on DeviceAccess, we could move it there.
// it is required from appliations and DoocsAdapter
// Maybe also ControlsystemAdapter in future, but not currently.

#include <type_traits>

#include <ChimeraTK/OneDRegisterAccessor.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

namespace ChimeraTK {

  /**
   *  generic header for opaque struct handling
   */
  struct OpaqueStructHeader {
    uint8_t encodingId = 0; // 0: default (little endian) binary
    uint8_t reserved = 0;
    uint16_t nameSpaceId = 0; // 0: ChimeraTK namespace
    uint32_t dataTypeId = 0;
    uint32_t totalLength = 0; // 0: unknown/not set. length includes header.
    OpaqueStructHeader(uint32_t dataTypeId_) : dataTypeId(dataTypeId_) {}
  };

  /**
   * Provides interface to a struct that is mapped onto a 1D array of ValType
   * The struct must be derived from OpaqueStructHeader.
   * Variable-length structs are supported, as long as they do not grow beyond the size of the given 1D array.
   */
  template<class StructHeader>
  class MappedStruct {
   public:
    /// call with initData=false if data already contains valid struct data.
    explicit MappedStruct(unsigned char* data, size_t dataLen, bool initData = true) {
      static_assert(std::is_base_of<OpaqueStructHeader, StructHeader>::value,
          "MappedStruct expects StructHeader to implement OpaqueStructHeader");
      _data = data;
      _dataLen = dataLen;
      if(initData) {
        memset(_data, 0, _dataLen);
        _header = new(_data) StructHeader;
      }
      else {
        _header = reinterpret_cast<StructHeader*>(_data);
      }
    }
    /// convenience: same as above but for OneDRegisterAccessor
    explicit MappedStruct(ChimeraTK::OneDRegisterAccessor<unsigned char>& accToData, bool initData = true)
    : MappedStruct(accToData.data(), accToData.getNElements(), initData) {}
    /// provided for convenience, if MappedStruct should allocate the data vector
    MappedStruct() {
      _allocate = true;
      realloc(sizeof(StructHeader));
    }
    unsigned char* data() { return _data; }
    size_t dataLen() const { return _dataLen; }
    /// e.g. for setting meta data
    StructHeader& header() { return *_header; }

   protected:
    void realloc(size_t newLen) {
      _dataLen = newLen;
      _allocatedBuf.resize(_dataLen);
      _data = _allocatedBuf.data();
      _header = new(_data) StructHeader;
    }

    bool _allocate = false;
    std::vector<unsigned char> _allocatedBuf; // used only if _allocate=true
    StructHeader* _header;
    unsigned char* _data = nullptr; // pointer to data for header and struct content
    size_t _dataLen = 0;
  };

  // example
  // TODO - make test case instead
  struct AStruct : public OpaqueStructHeader {
    int a;
    float x, y;
    AStruct() : OpaqueStructHeader(123) {}
  };
  void test1() {
    unsigned len = 100;
    std::vector<unsigned char> buf(len);
    MappedStruct<AStruct> ms(buf.data(), len, true);
    auto h = ms.header();
    h.x = 4.;
  }

  // application to Image, as struct

  /// values for the options bit field of ImgHeader
  const unsigned ImgOpt_RowMajor = 1;

  /// Image header defined like doocs::IMH, except we use defined-size types
  struct ImgHeader : public OpaqueStructHeader {
    ImgHeader() : OpaqueStructHeader(10) {}
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t aoi_width = 0;
    uint32_t aoi_height = 0;
    uint32_t x_start = 0;
    uint32_t y_start = 0;
    uint32_t bpp = 0;
    uint32_t ebitpp = 0;
    uint32_t hbin;
    uint32_t vbin;
    uint32_t source_format;
    uint32_t image_format;
    uint32_t frame = 0;
    uint32_t event = 0;
    float scale_x = 0;
    float scale_y = 0;
    float image_rotation = 0;
    float fspare2;
    float fspare3;
    float fspare4;
    uint32_t image_flags = 0;
    uint32_t channels = 0;
    uint32_t options = ImgOpt_RowMajor;
    uint32_t ispare4;
    uint32_t length = 0;
  };

  /**
   * provides convenient matrix-like access for MappedImage
   */
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
   * interface to an image that is mapped onto a 1D array of ValType
   */
  class MappedImage : public MappedStruct<ImgHeader> {
   public:
    using MappedStruct<ImgHeader>::MappedStruct;

    /// needs to be called after construction. corrupts all data except header.
    void setShape(unsigned width, unsigned height, unsigned channels = 1, unsigned bpp = 1) {
      size_t totalLen = sizeof(ImgHeader) + (size_t)width * height * bpp;
      if(!_allocate) {
        assert(totalLen <= _dataLen);
      }
      else {
        realloc(totalLen);
      }
      _header->totalLength = totalLen;
      _header->width = width;
      _header->height = height;
      _header->channels = channels;
      _header->bpp = bpp;
    }

    /// returns an ImgView object which can be used like a matrix. The ImgView becomes invalid at next setShape call.
    template<typename UserType, unsigned OPTIONS = ImgOpt_RowMajor>
    ImgView<UserType, OPTIONS> interpretedView() {
      assert(_header->channels > 0 && "call setShape() before interpretedView()!");
      assert(_header->bpp == _header->channels * sizeof(UserType) &&
          "choose correct bpp and channels value before conversion!");
      assert((_header->options & ImgOpt_RowMajor) == (OPTIONS & ImgOpt_RowMajor) &&
          "inconsistent data ordering col/row major");
      ImgView<UserType, OPTIONS> ret;
      ret._h = _header;
      ret._vec = reinterpret_cast<UserType*>(_data + sizeof(ImgHeader));
      return ret;
    }
  };

} // namespace ChimeraTK
