// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DoocsAdapter.h"
#include <doocs/Server.h>

ChimeraTK::DoocsAdapter doocsAdapter;

/*********************************************************************************************************************/

int main(int argc, char* argv[]) {
  doocsAdapter.createServer()->run(argc, argv);
  return 0;
}

/*********************************************************************************************************************/
