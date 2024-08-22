// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <string>

namespace ChimeraTK {

  /** Find the last slash and return the subsring behind it.
   */
  inline std::string basenameFromAddress(std::string const& doocsAddress) {
    // find first slash
    auto slashPosition = doocsAddress.rfind('/');
    // no slash found: return the whole string
    if(slashPosition == std::string::npos) {
      return doocsAddress;
    }
    return doocsAddress.substr(slashPosition + 1);
  }

} // namespace ChimeraTK
