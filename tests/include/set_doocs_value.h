#ifndef set_doocs_value_h
#define set_doocs_value_h

#include <d_fct.h>

// a convenience function to call set() on a D_fct, which is doing the real call
// overloaded/ not overloaded set_value, fill history, call set_and_arcive etc.
template<class DOOCS_T, class T>
int set_doocs_value(DOOCS_T& property, T value) {
  EqAdr adr;
  EqData src, dest;
  src.set(value);
  property.set(&adr, &src, &dest, nullptr);
  return dest.error();
}

#endif // set_doocs_value_h
