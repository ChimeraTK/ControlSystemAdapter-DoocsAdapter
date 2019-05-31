#ifndef __DOOCS_PROCESS_SCALAR_H__
#define __DOOCS_PROCESS_SCALAR_H__

#include "DoocsUpdater.h"
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include <ChimeraTK/DataConsistencyGroup.h>
#include <boost/shared_ptr.hpp>
#include <chrono>
#include <d_fct.h>
#include <eq_fct.h>
#include <string>

namespace ChimeraTK {

  /** The DoocsProcessScalar has three template parameters:
   *  \li \c T, The primitive value type of the ChimeraTK process variable
   *  \li \c DOOCS_T, The Doocs type which is used
   */
  template<typename T, typename DOOCS_T>
  class DoocsProcessScalar : public DOOCS_T, public boost::noncopyable {
   public:
    void updateDoocsBuffer(TransferElementID transferElementId) {
      // FIXME: A first  implementation is checking the data consistency here. Later this should be
      // before calling this function because calling this function through a function pointer is
      // comparatively expensive.
      // Only check the consistency group if there is a macro pulse number associated.
      if (_macroPulseNumberSource && !_consistencyGroup.update(transferElementId)){
        // data is not consistent (yet). Don't update the Doocs buffer.
//        std::cout << "no consistency in transferElement " << (transferElementId== _macroPulseNumberSource->getId()?"MacroPulse":"Scalar")<<  ", Name " << _processScalar->getName() <<", macropulse version " << std::string(_macroPulseNumberSource->getVersionNumber())
//                  << ", scalar version " <<  std::string(_processScalar->getVersionNumber()) << std::endl;
        return;
      }
//      if (_macroPulseNumberSource ){
//        std::cout << "consistency found in transferElement " << (transferElementId== _macroPulseNumberSource->getId()?"MacroPulse":"Scalar")<< ", Name " << _processScalar->getName()
//                  << ", version " << std::string(_processScalar->getVersionNumber()) << std::endl;
//      }else{
//          std::cout << "no macropulse number set for " << _processScalar->getName() << std::endl;
//      }

      // Note: we already own the location lock by specification of the
      // DoocsUpdater
      auto data = _processScalar->accessData(0);
      // we must not call set_and_archive if there is no history (otherwise it
      // will be activated), but we have to if it is there. -> Abstraction,
      // please!
      if(this->get_histPointer()) {
        this->set_and_archive(data);
      }
      else {
        this->set_value(data);
      }
      if(_publishZMQ) {
        dmsg_info info;
        memset(&info, 0, sizeof(info));
        auto sinceEpoch = _processScalar->getVersionNumber().getTime().time_since_epoch();
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(sinceEpoch);
        info.sec = time.count() / 1000000;
        info.usec = time.count() % 1000000;
        info.ident = _macroPulseNumberSource->accessData(0);
        this->send(&info);
      }
    }

    DoocsProcessScalar(EqFct* eqFct, std::string doocsPropertyName,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
        : DOOCS_T(eqFct, doocsPropertyName.c_str()), _processScalar(processScalar), _doocsUpdater(updater), _eqFct(eqFct){
      if(processScalar->isReadable()) {
        updater.addVariable(ChimeraTK::ScalarRegisterAccessor<T>(processScalar), eqFct,
            std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this, processScalar->getId()));
        _consistencyGroup.add(processScalar);
      }
    }

    DoocsProcessScalar(std::string doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
    : DOOCS_T(doocsPropertyName.c_str(), eqFct), _processScalar(processScalar), _doocsUpdater(updater), _eqFct(eqFct) {
      if(processScalar->isReadable()) {
        updater.addVariable(ChimeraTK::ScalarRegisterAccessor<T>(processScalar), eqFct,
            std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this, processScalar->getId()));
        _consistencyGroup.add(processScalar);
      }
    }

    /**
     * Override the Doocs set method which is triggered by the RPC calls.
     */
    void set(EqAdr* adr, EqData* data1, EqData* data2, EqFct* eqfct) override {
      // only assign the value if the variable is writeable
      // Otherwise the content displayed by Doocs and the value in the application
      // are inconsistent
      if(_processScalar->isWriteable()) {
        DOOCS_T::set(adr, data1, data2, eqfct);
        // let the DOOCS_T set function do all the dirty work and use the
        // get_value function afterwards to get the already assigned value
        _processScalar->accessData(0) = this->value();
        _processScalar->write();
      }
    }

    /**
     * Override the Doocs auto_init() method, which is called after initialising
     * the value of the property from the config file.
     */
    void auto_init(void) override {
      DOOCS_T::auto_init();
      // send the current value to the device
      if(_processScalar->isWriteable()) {
        _processScalar->accessData(0) = DOOCS_T::value();
        _processScalar->write();
      }
    }

    void publishZeroMQ() { _publishZMQ = true; }

    void setMacroPulseNumberSource(boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> macroPulseNumberSource) {
        if(_processScalar->isReadable()) {
          _macroPulseNumberSource = macroPulseNumberSource;
          _consistencyGroup.add(macroPulseNumberSource);
          _doocsUpdater.addVariable(ChimeraTK::ScalarRegisterAccessor<int64_t>(macroPulseNumberSource), _eqFct,
              std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this, macroPulseNumberSource->getId()));
        }
    }

   protected:
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> _processScalar;
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<int64_t>> _macroPulseNumberSource;
    DataConsistencyGroup _consistencyGroup;
    DoocsUpdater & _doocsUpdater; // store the reference to the updater. We need it when adding the macro pulse number
    EqFct * _eqFct; // We need it when adding the macro pulse number
    bool _publishZMQ{false};
  };

} // namespace ChimeraTK

#endif // __DOOCS_PROCESS_SCALAR_H__
