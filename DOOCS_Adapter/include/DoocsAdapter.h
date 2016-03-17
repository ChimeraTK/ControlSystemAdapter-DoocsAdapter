#ifndef _MTCA4U_DOOCS_ADAPTER_H_
#define _MTCA4U_DOOCS_ADAPTER_H_

#include "DoocsPVFactory.h"
#include "CSAdapterEqFct.h"

namespace mtca4u{

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

/** Concenience macros to reduce the amount of boiler plate code.
 */
#define BEGIN_DOOCS_SERVER( SERVER_NAME, EQ_CODE )\
  const char * object_name = SERVER_NAME;	  \
  int csAdapterEqCode = EQ_CODE;\
  EqFct * eq_create (int eq_code, void *){	\
    static mtca4u::DoocsAdapter doocsAdapter;


#define END_DOOCS_SERVER()\
    if (eq_code == csAdapterEqCode){\
      return new mtca4u::CSAdapterEqFct(eq_code, doocsAdapter.getControlSystemPVManager());\
    }else{\
      return NULL;\
    }\
  }

}//namespace mtca4u

#endif // _MTCA4U_DOOCS_ADAPTER_H_
