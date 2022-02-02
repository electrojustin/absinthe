#include <stdarg.h>

#include "lib/string.h"

namespace lib {

namespace {

constexpr char hexits[17] = "0123456789ABCDEF";

// Helper function for converting decimal numbers to strings.
int convert_decimal(char* dest, int64_t to_print) {
	int written = 0;
	int index = 19;
	uint8_t digits[20] = {0};
	if (to_print < 0) {
		dest[written] = '-';
		written++;
		to_print *= -1;
	} else if (!to_print) {
		dest[written] = '0';
		return 1;
	}
	while (to_print) {
		digits[index] = to_print % 10;
		to_print /= 10;
		index--;
	}
	index++;
	for (index; index < 20; index++) {
		dest[written] = hexits[digits[index]];
		written++;
	}
	return written;
}

// Helper function for converting hexadecimal numbers to strings.
int convert_hex(char* dest, uint64_t to_print) {
	*dest = '0';
	dest++;
	*dest = 'x';
	dest++;
	for (int i = 60; i >= 0; i-=4) {
		*dest = hexits[(to_print >> i) & 0xF];
		dest++;
	}
	return 18;
}

} // namespace

int sprintnk(char* dest, uint64_t max_chars, const char* format, ...) {
	va_list parameters;
	va_start(parameters, format);
	return vsprintnk(dest, max_chars, format, parameters);
}

int vsprintnk(char* dest, uint64_t max_chars, const char* format, va_list parameters) {
	int index = 0;
	int written = 0;
	while (format[index]) {
		if (written + 1>= max_chars) {
			return -1;
		}

		if (format[index] != '%') {
			dest[written] = format[index];
			written++;
			index++;
			continue;
		}

		index++;
		if (written + 1>= max_chars || !format[index]) {
			return -1;
		}

		char* string_buf = nullptr;
		switch (format[index]) {
			case '%':
				dest[written] = '%';
				written++;
				break;
			case 'c':
				dest[written] = (char)va_arg(parameters, int);
				written++;
				break;
			case 's':
				string_buf = (char*)va_arg(parameters, char*);
				while (*string_buf) {
					if (written >= max_chars) {
						return -1;
					}
					dest[written] = *string_buf;
					written++;
					string_buf++;
				}
				break;
			case 'd':
				if (written + 11 >= max_chars) {
					return -1;
				}
				written += convert_decimal(dest+written, va_arg(parameters, int64_t));
				break;
			case 'x':
				if (written + 11 >= max_chars) {
					return -1;
				}
				written += convert_hex(dest+written, va_arg(parameters, uint64_t));
				break;
			default:
				return -1;
		}
		index++;
	}

	dest[written] = 0;
	written++;

	return written;
}

} //namespace lib
