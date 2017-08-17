#ifndef GET_ALL_VARIABLENAMES_H
#define GET_ALL_VARIABLENAMES_H

#include <set>
#include <string>
#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>

namespace ChimeraTK{

  ///convenience function to get all variable names from the CS adapter as a std::set
  ///(needed for instance for the variable mapper)
  std::set< std::string > getAllVariableNames( boost::shared_ptr<ControlSystemPVManager> csManager );

} //  namespace ChimeraTK

#endif //GET_ALL_VARIABLENAMES_H
