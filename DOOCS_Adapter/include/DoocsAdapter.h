#ifndef _MTCA4U_DOOCS_ADAPTER_H_
#define _MTCA4U_DOOCS_ADAPTER_H_

#include "DoocsPVFactory.h"

namespace mtca4u{

  /** The main adapter class. With this tool the EqFct should shrink to about 4 lines of code (plus boiler plate).
   */
class DoocsAdapter{
 public:
  DoocsAdapter(EqFct *eqFct);
  boost::shared_ptr<DevicePVManager> & getDevicePVManager();
  //boost::shared_ptr<ControlSystemPVManager> getControlSystemPVManager();

  /** Call this function after you created your business logic. All ProcessVariables are know
   *  to the PVManagers, so they can be registered with Doocs now.
   */
  void registerProcessVariablesInDoocs();

  /** Receive all variables that have been update by the business logic on the control system side.
   */
  void receiveAll();
  
  //boost::shared_ptr< ControlSystemSynchronizationUtility > getSyncUtility();

 protected:
  boost::shared_ptr<ControlSystemPVManager> _controlSystemPVManager;
  boost::shared_ptr<DevicePVManager> _devicePVManager;

  /** We have to store and hold all doocs properties. They only go out of scope when the adapter
   *  goes out of scope, which should be the destructor of EqFct.
   */
  std::vector< boost::shared_ptr<D_fct> > _doocsProperties;

  /** Hold a copy of the syncUtility. It holds the ReceiveListeners and is needed in reveiveAll().
   */
  boost::shared_ptr< ControlSystemSynchronizationUtility > _syncUtility;

  /** We need the EqFct for the factory, so we store it.
   */
  EqFct * _eqFct;
};

}//namespace mtca4u

#endif // _MTCA4U_DOOCS_ADAPTER_H_
