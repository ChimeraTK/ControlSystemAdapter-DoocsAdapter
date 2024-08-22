// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

/* These functions have to be defined when linking to the server library.
   These are the minimal implementations which don't do anything.
*/

/* These functions are never used in a real server, so they never can be
   called and are excluded from the coverage report.
 */

#include <eq_fct.h>

// LCOV_EXCL_START

inline EqFct* eqCreate(int /*eq_code*/, void*) {
  return nullptr;
}
inline void eqInitEpilog() {}
inline void eqInitProlog() {}
inline void refreshEpilog() {}
inline void refreshProlog() {}
inline void interruptUsr1Prolog(int) {}
inline void interruptUsr1Epilog(int) {}
inline void interruptUsr2Prolog() {}
inline void interruptUsr2Epilog() {}
inline void postInitProlog() {}
inline void postInitEpilog() {}
inline void eqCancel() {}

// LCOV_EXCL_STOP
