#ifndef __DOOCS_SPECTRUM_H__
#define __DOOCS_SPECTRUM_H__

#include <D_spectrum.h>
#include <boost/noncopyable.hpp>

#include "DoocsUpdater.h"
#include "splitStringAtFirstSlash.h"
#include <ChimeraTK/NDRegisterAccessor.h>

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
     */
    DoocsSpectrum(EqFct* eqFct, std::string const& doocsPropertyName,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& processArray, DoocsUpdater& updater,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& startAccessor,
        boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> const& incrementAccessor);

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
    void updateDoocsBuffer();

    // callback function after the start or increment variables have changed
    void updateParameters();

    void publishZeroMQ() { publishZMQ = true; }

    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
      _macroPulseNumberSource = macroPulseNumberSource;
    }

   protected:
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _processArray;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _startAccessor;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<float>> _incrementAccessor;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
    bool publishZMQ{false};

    // Internal function which copies the content from the DOOCS container into
    // the ChimeraTK ProcessArray and calls the send method. Factored out to allow
    // unit testing.
    void sendToDevice();
  };

} // namespace ChimeraTK

#endif // __DOOCS_SPECTRUM_H__
