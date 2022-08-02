// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "D_textUnifier.h"

// just call the constructor without history (the only one)
D_textUnifier::D_textUnifier(EqFct* eqFct, std::string doocsPropertyName) : D_text(doocsPropertyName, eqFct) {}

void D_textUnifier::set_and_archive(const std::string& str, ArchiveStatus, doocs::Timestamp, doocs::EventId) {
  set_value(str);
}

D_hist* D_textUnifier::get_histPointer() {
  return nullptr;
}
