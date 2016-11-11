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

/** Concenience macros to reduce the amount of boiler plate code.
 * 
 *  TODO @todo These macros are deprecated. Use the ApplicationBase class instead!
 */
#define BEGIN_DOOCS_SERVER( SERVER_NAME, EQ_CODE )\
  const char * object_name = SERVER_NAME;	  \
  int csAdapterEqCode = EQ_CODE;\
  EqFct * eq_create (int eq_code, void *){	\
    static ChimeraTK::DoocsAdapter doocsAdapter;


#define END_DOOCS_SERVER()\
    if (eq_code == csAdapterEqCode){\
      return new ChimeraTK::CSAdapterEqFct(eq_code, doocsAdapter.getControlSystemPVManager());\
    }else{\
      return NULL;\
    }\
  }

#endif // _CHIMERATK_DOOCS_ADAPTER_H_
