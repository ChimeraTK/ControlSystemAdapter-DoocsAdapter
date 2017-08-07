#include "VariableMapper.h"

namespace ChimeraTK{
  VariableMapper & VariableMapper::getInstance(){
    static VariableMapper instance;
    return instance;
  }

  

} // namespace ChimeraTK

