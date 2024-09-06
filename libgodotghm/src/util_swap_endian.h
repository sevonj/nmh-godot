// https://stackoverflow.com/a/4956493

#ifndef UTIL_SWAP_ENDIAN
#define UTIL_SWAP_ENDIAN

#include <climits>

template <typename T>
T swap_endian(T u) {
  static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

  union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;

  source.u = u;

  for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1];

  return dest.u;
}

#endif  // UTIL_SWAP_ENDIAN