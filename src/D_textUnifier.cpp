#include "D_textUnifier.h"

// just call the constructor without history (the only one)
D_textUnifier::D_textUnifier(EqFct* eqFct, std::string doocsPropertyName) : D_text(doocsPropertyName, eqFct) {}

void D_textUnifier::set_and_archive(
    const std::string& str, eq_sts_codes /*status*/, u_int /*seconds*/, u_int /*microseconds*/) {
  set_value(str);
}

D_hist* D_textUnifier::get_histPointer() {
  return nullptr;
}
