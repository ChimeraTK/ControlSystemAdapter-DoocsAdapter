#ifndef __DOOCS_SPECTRUM_H__
#define __DOOCS_SPECTRUM_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include "DoocsUpdater.h"
#include "splitStringAtFirstSlash.h"
#include <ChimeraTK/NDRegisterAccessor.h>
#include <ChimeraTK/ScalarRegisterAccessor.h> // needed for the macro pulse number
#include <ChimeraTK/DataConsistencyGroup.h>

#include <iostream>

// Just declare the EqFct class. We only need the pointer in this header.
class EqFct;

namespace ChimeraTK {

  class DoocsSpectrum : public D_spectrum, public boost::noncopyable {
   public:
    /** The constructor expects an NDRegisterAccessor of float, which usually will
     * be a decorator to the implementation type. The decorator cannot be
     * generated in the constructor because the ProcessVariable aka
     * TransferElement does not know about it's size, which is needed by the
     * D_spectrum constructor. This is not a big drawback because the properties
     * are greated by a factory function anyway.
     *
     * This version of the constructor shall be used for writeable spectra, as they
     * do not require mulitple buffers but should be saved to and restored from a file
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
     * are greated by a factory function anyway.
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

    // call this function after a tranfer element has requested it.
    void updateDoocsBuffer(TransferElementID transferElementId);

    // callback function after the start or increment variables have changed
    void updateParameters();

    void publishZeroMQ() { publishZMQ = true; }

    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource);

    void write(std::fstream& fptr) override;

    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _processArray;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _startAccessor;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _incrementAccessor;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
    DataConsistencyGroup _consistencyGroup;
    DoocsUpdater& _doocsUpdater; // store the reference to the updater. We need it when adding the macro pulse number
    EqFct* _eqFct;               // We need it when adding the macro pulse number
    bool publishZMQ{false};
    size_t nBuffers;

    // Internal function which copies the content from the DOOCS container into
    // the ChimeraTK ProcessArray and calls the send method. Factored out to allow
    // unit testing.
    void sendToDevice();

    // Flag whether the value has been modified since the content has been saved to disk the last time (see write()).
    bool modified{false};

    bool _doocsSuccessfullyUpdated{true}; // to detect data losses
  };

} // namespace ChimeraTK

#endif // __DOOCS_SPECTRUM_H__
