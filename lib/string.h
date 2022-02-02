#ifndef LIB_STRING_H
#define LIB_STRING_H

#include <stdint.h>

namespace lib {

// Formats supported:
// %c = character
// %s = string
// %d = decimal formatted integer
// %x = hex formatted integer (unsigned)
// %% = %
int sprintnk(char* dest, uint64_t max_chars, const char* format, ...);

int vsprintnk(char* dest, uint64_t max_chars, const char* format, va_list parameters);

} // namespace lib

#endif
