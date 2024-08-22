// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <d_fct.h>

// a convenience function to call set() on a D_fct, which is doing the real call
// overloaded/ not overloaded set_value, fill history, call set_and_arcive etc.
template<class DOOCS_T, class T>
int set_doocs_value(DOOCS_T& property, T value) {
  EqAdr adr;
  doocs::EqData src, dest;
  src.set(value);
  property.get_eqfct()->lock();
  property.set(&adr, &src, &dest, nullptr);
  property.get_eqfct()->unlock();
  return dest.error();
}
