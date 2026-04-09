// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Utilities.h"

/********************************************************************************************************************/

std::string getAbsoluteSource(std::string source, const std::string& locationName) {
  if(source[0] == '/') {
    return source;
  }
  return std::string("/") + locationName + "/" + source;
}

/********************************************************************************************************************/
