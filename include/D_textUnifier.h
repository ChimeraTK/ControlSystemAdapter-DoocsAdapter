#ifndef D_TEXTUNIFIER_H
#define D_TEXTUNIFIER_H

#include <d_fct.h>
#include <eq_sts_codes.h>

/** A compatibility class that adds the constructor which would create a history,
 *  and the set_and_archive function to the D_text interface. As this does not
 *  have history it just calls the non-history equivalent.
 *  This class was introduces to avoid template specialisations in DoocsProcessScalar.
 *  (Local specialisations of functions don't work because one would have to
 */
struct D_textUnifier : public D_text {
  using D_text::D_text;
  D_textUnifier(EqFct* eqFct, std::string doocsPropertyName);

  /// \todo FIXE: eq_sts_codes will be deprecated, but currently is require to compile with the latest pubilshed doocs release
  /// It will be changed to ArchiveStatus.
  void set_and_archive(const std::string& str, eq_sts_codes status = sts_ok, u_int seconds = 0, u_int microseconds = 0);
  D_hist* get_histPointer();
};

#endif // D_TEXTUNIFIER_H
