// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "getAllVariableNames.h"

namespace ChimeraTK {

  std::set<std::string> getAllVariableNames(boost::shared_ptr<ControlSystemPVManager> csManager) {
    std::set<std::string> output;
    for(auto& pv : csManager->getAllProcessVariables()) {
      output.insert(pv->getName());
    }

    return output;
  }

} //  namespace ChimeraTK
