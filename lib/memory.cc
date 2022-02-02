#include "lib/memory.h"

namespace lib {

void memcpy(uint8_t* dest, uint8_t* src, size_t size) {
	uint64_t* dest_word = (uint64_t*)dest;
	uint64_t* src_word = (uint64_t*)src;
	for (uint64_t i = 0; i < (size >> 3); i++) {
		*dest_word = *src_word;
		dest_word++;
		src_word++;
	}
	dest = (uint8_t*)dest_word;
	src = (uint8_t*)src_word;
	for (uint64_t i = 0; i < (size & 7); i++) {
		*dest = *src;
		dest++;
		src++;
	}
}

void memset(uint8_t* dest, uint8_t value, size_t size) {
	if (size >= 8) {
		uint64_t word_value = value;
		word_value |= (word_value <<  8) | (word_value << 16) |
			      (word_value << 24) | (word_value << 32) |
			      (word_value << 40) | (word_value << 48);
		uint64_t* dest_word = (uint64_t*)dest;
		for (uint64_t i = 0; i < (size >> 3); i++) {
			*dest_word = word_value;
			dest_word++;
		}

		dest = (uint8_t*)dest_word;
	}
	for (uint64_t i = 0; i < (size & 7); i++) {
		*dest = value;
		dest++;
	}
}

void bzero(uint8_t* dest, size_t size) {
	memset(dest, 0, size);
}

} // namespace lib
