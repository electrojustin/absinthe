#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#include <stdint.h>

namespace lib {

void memcpy(uint8_t* dest, uint8_t* src, size_t size);

void memset(uint8_t* dest, uint8_t value, size_t size);

void bzero(uint8_t* dest, size_t size);

} // namespace lib

#endif
