#ifndef __DOOCS_PROCESS_SCALAR_H__
#define __DOOCS_PROCESS_SCALAR_H__

#include <string>
#include <ChimeraTK/ScalarRegisterAccessor.h>
#include "DoocsUpdater.h"
#include <boost/shared_ptr.hpp>
#include <d_fct.h>
#include <eq_fct.h>

namespace ChimeraTK {

  /** The DoocsProcessScalar has three template parameters:
   *  \li \c T, The primitive value type of the ChimeraTK process variable
   *  \li \c DOOCS_T, The Doocs type which is used
   */
  template<typename T, typename DOOCS_T>
  class DoocsProcessScalar : public DOOCS_T, public boost::noncopyable {
   public:
    void updateDoocsBuffer() {
      // Note: we already own the location lock by specification of the DoocsUpdater
      auto data = _processScalar->accessData(0);
      // we must not call set_and_archive if there is no history (otherwise it will be activated), but we have to if it
      // is there. -> Abstraction, please!
      if(this->get_histPointer()) {
        this->set_and_archive(data);
      }
      else {
        this->set_value(data);
      }
    }

    DoocsProcessScalar(EqFct* eqFct, std::string doocsPropertyName,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
    : DOOCS_T(eqFct, doocsPropertyName.c_str()), _processScalar(processScalar) {
      if(processScalar->isReadable()) {
        updater.addVariable(ChimeraTK::ScalarRegisterAccessor<T>(processScalar), eqFct,
            std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this));
      }
    }

    DoocsProcessScalar(std::string doocsPropertyName, EqFct* eqFct,
        boost::shared_ptr<typename ChimeraTK::NDRegisterAccessor<T>> const& processScalar, DoocsUpdater& updater)
    : DOOCS_T(doocsPropertyName.c_str(), eqFct), _processScalar(processScalar) {
      if(processScalar->isReadable()) {
        updater.addVariable(ChimeraTK::ScalarRegisterAccessor<T>(processScalar), eqFct,
            std::bind(&DoocsProcessScalar<T, DOOCS_T>::updateDoocsBuffer, this));
      }
    }

    /**
     * Override the Doocs set method which is triggered by the RPC calls.
     */
    void set(EqAdr* adr, EqData* data1, EqData* data2, EqFct* eqfct) override {
      // only assign the value if the variable is writeable
      // Otherwise the content displayed by Doocs and the value in the application are inconsistent
      if(_processScalar->isWriteable()) {
        DOOCS_T::set(adr, data1, data2, eqfct);
        // let the DOOCS_T set function do all the dirty work and use the get_value function afterwards to get the
        // already assigned value
        _processScalar->accessData(0) = this->value();
        _processScalar->write();
      }
    }

    /**
     * Override the Doocs auto_init() method, which is called after initialising the value of
     * the property from the config file.
     */
    void auto_init(void) {
      DOOCS_T::auto_init();
      // send the current value to the device
      if(_processScalar->isWriteable()) {
        _processScalar->accessData(0) = DOOCS_T::value();
        _processScalar->write();
      }
    }

   protected:
    boost::shared_ptr<ChimeraTK::NDRegisterAccessor<T>> _processScalar;
  };

} // namespace ChimeraTK

#endif // __DOOCS_PROCESS_SCALAR_H__
