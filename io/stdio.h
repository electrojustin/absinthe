#ifndef IO_STDIO_H
#define IO_STDIO_H

#include <stdint.h>

namespace io {

void putc(char c);

void puts(const char* s);

int printk(const char* format, ...);

void print_stack_trace();

} // namespace io

#endif
