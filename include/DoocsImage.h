// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DoocsUpdater.h"
#include "PropertyBase.h"

#include <ChimeraTK/MappedImage.h>

#include <eq_fct.h>

namespace ChimeraTK {

  /*
   * concept for image in ApplicatonCore and mapping to DOOCS
   *
   * - Use 1D register accessor with fixed size (upper limit) to store image,
   *   consisting of header and data. This ensures data consistency.
   * - flexibility with regard of the UserType of the array is achieved via ImgView which does reinterpret_cast of the
   * byte array
   * - provide mapping to some DOOCS formats
   *
   */

  /// implements DOOCS-specific mapping for MappedImage
  struct MappedDoocsImg : public MappedImage {
    using MappedImage::MappedImage;

    /// Overwrites headerOut and returns pointer to internal data, without header
    /// The DOOCS image format is selected based on previously set header info, i.e. channels and bpp.
    unsigned char* asDoocsImg(IMH* headerOut);
  };

  /**
   * The class used by DoocsUpdater to hand images over to Doocs
   */
  class DoocsImage : public D_imagec, public PropertyBase {
   public:
    DoocsImage(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> const& processArray, DoocsUpdater& updater);

    IMH imh{};
    IMH* getIMH() { return &imh; }

   protected:
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<uint8_t>> _processArray;
  };

} // namespace ChimeraTK
