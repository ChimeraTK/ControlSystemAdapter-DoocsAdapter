// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <d_fct.h>

/** A compatibility class that adds the constructor which would create a history,
 *  and the set_and_archive function to the D_text interface. As this does not
 *  have history it just calls the non-history equivalent.
 *  This class was introduces to avoid template specialisations in DoocsProcessScalar.
 *  (Local specialisations of functions don't work because one would have to
 */
struct DTextUnifier : public D_text {
  using D_text::D_text;
  DTextUnifier(EqFct* eqFct, const std::string& doocsPropertyName);

  void setAndArchive(const std::string& str, ArchiveStatus status, doocs::Timestamp timestamp, doocs::EventId eventId);
  static D_hist* getHistPointer();
};
