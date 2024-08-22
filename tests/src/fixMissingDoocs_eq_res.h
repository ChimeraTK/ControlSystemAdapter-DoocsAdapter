// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

// Due to an API change bug in DOOCS 23.12.0 the comp_code enum with the error values returned by EqCall has been
// bemoved from the public API. This fixes the test until the DOOCS API is adapted in the next release by copying the
// code from the private interface.
namespace fixme {
  enum comp_code {
    transport_error = -2,   // can reach remote server but not the property
    transaction_error = -1, // can not reach remote server at all
    ok = 0,
    data_error = 1, // d_error set in source  };
  };
} // namespace fixme
