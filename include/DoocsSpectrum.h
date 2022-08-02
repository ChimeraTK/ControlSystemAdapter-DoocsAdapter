// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"
#include <D_spectrum.h>

#include <ChimeraTK/NDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

#include <iostream>

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace ChimeraTK {

  class DoocsSpectrum : public D_spectrum, public boost::noncopyable, public PropertyBase {
   public:
    /** The constructor expects an NDRegisterAccessor of float, which usually will
     * be a decorator to the implementation type. The decorator cannot be
     * generated in the constructor because the ProcessVariable aka
     * TransferElement does not know about it's size, which is needed by the
     * D_spectrum constructor. This is not a big drawback because the properties
     * are greated by a factory function anyway.
     *
     * This version of the constructor shall be used for writeable spectra, as they
     * do not require multiple buffers but should be saved to and restored from a file
     * for persistency.
     */
    DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& startAccessor,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& incrementAccessor);

    /** The constructor expects an NDRegisterAccessor of float, which usually will
     * be a decorator to the implementation type. The decorator cannot be
     * generated in the constructor because the ProcessVariable aka
     * TransferElement does not know about it's size, which is needed by the
     * D_spectrum constructor. This is not a big drawback because the properties
     * are generated by a factory function anyway.
     *
     * This version of the constructor shall be used for read-only spectra, as they
     * do not need to be persisted but might require multiple buffers for short-term
     * history.
     */
    DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& startAccessor,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& incrementAccessor, size_t numberOfBuffers);

    /**
     * Overload the set function which is called by DOOCS to inject sending to the
     * device.
     */
    void set(EqAdr* eqAdr, EqData* data1, EqData* data2, EqFct* eqFct) override;

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init(void) override;

    /// call this function after a tranfer element has requested it.
    void updateDoocsBuffer(const TransferElementID& transferElementId) override;

    void write(std::ostream& s) override;

   protected:
    void addParameterAccessors();
    /// callback function after the start or increment variables have changed
    void updateParameters();
    /// Internal function which copies the content from the DOOCS container into
    /// the ChimeraTK ProcessArray and calls the send method. Factored out to allow
    /// unit testing.
    void sendToDevice(bool getLock);

   public:
    /// Flag whether the value has been modified since the content has been saved to disk the last time (see write()).
    bool modified{false};

   protected:
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _processArray;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _startAccessor;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _incrementAccessor;
    size_t nBuffers;
  };

} // namespace ChimeraTK
