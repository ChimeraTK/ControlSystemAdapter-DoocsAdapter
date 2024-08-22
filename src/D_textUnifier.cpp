// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "D_textUnifier.h"

// just call the constructor without history (the only one)
DTextUnifier::DTextUnifier(EqFct* eqFct, const std::string& doocsPropertyName) : D_text(doocsPropertyName, eqFct) {}

void DTextUnifier::setAndArchive(const std::string& str, ArchiveStatus, doocs::Timestamp, doocs::EventId) {
  set_value(str);
}

D_hist* DTextUnifier::getHistPointer() {
  return nullptr;
}
