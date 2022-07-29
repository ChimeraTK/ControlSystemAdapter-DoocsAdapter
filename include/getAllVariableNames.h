// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>
#include <set>
#include <string>

namespace ChimeraTK {

  /// convenience function to get all variable names from the CS adapter as a
  /// std::set (needed for instance for the variable mapper)
  std::set<std::string> getAllVariableNames(boost::shared_ptr<ControlSystemPVManager> csManager);

} //  namespace ChimeraTK
