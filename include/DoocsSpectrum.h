// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DoocsAdapter.h"
#include "DoocsUpdater.h"

#include <ChimeraTK/NDRegisterAccessor.h>

#include <boost/noncopyable.hpp>

#include <D_spectrum.h>

#include <iostream>

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace ChimeraTK {

  class DoocsSpectrum : public D_spectrum, public boost::noncopyable, public PropertyBase {
   public:
    /** Struct to hold EGU axis configuration from XML */
    struct AxisConfig {
      std::string label;
      int logarithmic{};
      float start{};
      float stop{};
    };

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
        DataConsistencyGroup::MatchingMode matchingMode,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> startAccessor,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> incrementAccessor);

    /** The constructor expects an NDRegisterAccessor of float, which usually will
     * be a decorator to the implementation type. The decorator cannot be
     * generated in the constructor because the ProcessVariable aka
     * TransferElement does not know about it's size, which is needed by the
     * D_spectrum constructor. This is not a big drawback because the properties
     * are created by a factory function anyway.
     *
     * This version of the constructor shall be used for read-only spectra, as they
     * do not need to be persisted but might require multiple buffers for short-term
     * history.
     */
    DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
        DataConsistencyGroup::MatchingMode matchingMode,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> startAccessor,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> incrementAccessor, size_t numberOfBuffers);

    /** Set the EGU axis configuration for this spectrum.
     *  Stores the config so it can be re-applied in auto_init() after the DOOCS framework
     *  overwrites it with persisted values from the .conf file.
     *  If set, the .EGU/.XEGU sub-properties are made read-only, preventing runtime modification. */
    void setAxisConfig(const std::string& axis, AxisConfig config);

    /**
     * Overload the set function which is called by DOOCS to inject sending to the
     * device.
     */
    void set(EqAdr* eqAdr, doocs::EqData* data1, doocs::EqData* data2, EqFct* eqFct) override;

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init() override;

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

    /// Re-apply EGU after .conf loading in auto_init(). EGU is set initially in
    /// DoocsPVFactory but then D_spectrum::read() loads .conf data.
    /// If EGU is defined in XML, force-sets via set_plot_y_value()/set_plot_x_value() and
    /// makes the .EGU/.XEGU sub-properties read-only. On mismatch with .conf values,
    /// logs a warning and overwrites with XML values.
    void applyAxisConfig();

   public:
    /// Flag whether the value has been modified since the content has been saved to disk the last time (see write()).
    bool modified{false};

   protected:
    OneDRegisterAccessor<float> _processArray;
    ScalarRegisterAccessor<float> _startAccessor;
    ScalarRegisterAccessor<float> _incrementAccessor;
    size_t _nBuffers;

    /// EGU axis configuration from the XML file, re-applied after auto_init()
    std::map<std::string, AxisConfig> _axisConfig;
  };

} // namespace ChimeraTK
