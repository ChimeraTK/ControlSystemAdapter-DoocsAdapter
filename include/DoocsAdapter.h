#ifndef _CHIMERATK_DOOCS_ADAPTER_H_
#define _CHIMERATK_DOOCS_ADAPTER_H_

#include "DoocsPVFactory.h"
#include "CSAdapterEqFct.h"

namespace ChimeraTK{

  /** The main adapter class. With this tool the EqFct should shrink to about 4 lines of code (plus boiler plate).
   */
class DoocsAdapter{
 public:
  DoocsAdapter();
  boost::shared_ptr<DevicePVManager> const & getDevicePVManager() const;
  boost::shared_ptr<ControlSystemPVManager> const & getControlSystemPVManager() const;

 protected:
  boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager;
  boost::shared_ptr<DevicePVManager> _devicePVManager;
};

}//namespace ChimeraTK

#endif // _CHIMERATK_DOOCS_ADAPTER_H_
