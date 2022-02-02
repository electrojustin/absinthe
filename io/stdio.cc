#include <stdarg.h>

#include "io/stdio.h"

#include "lib/string.h"

namespace io {

void putc(char c) {
	asm volatile(
		"add a0, zero, %0	\n" // Load character into a0
		"li a7, 0x01		\n" // Select extension 0x01 (putc)
		"ecall			\n" // ecall
		:
		: "r"(c)		    // %0
		: "a0", "a7");
}

void puts(const char* s) {
	while(*s) {
		putc(*s);
		s++;
	}
}


// Print formatted string.
// Formats supported:
// %c = character
// %s = string
// %d = decimal formatted integer
// %x = hex formatted integer (unsigned)
// %% = %
int printk(const char* format, ...) {
	char tmp_buf[512];
	va_list parameters;
	va_start(parameters, format);

	int written = lib::vsprintnk(tmp_buf, 512, format, parameters);
	puts(tmp_buf);

	return written;
}

extern "C" uint64_t KERNEL_STACK_TOP;

void print_stack_trace() {
	uint64_t frame_pointer;
	uint64_t return_address;
	asm volatile(
		"add %0, zero, ra	\n" // Get return address
		"add %1, zero, s0	\n" // Get frame pointer
		: "=r"(return_address),
		  "=r"(frame_pointer)
		:
		:);
	printk("Stack trace!\nAt %x\n", return_address-0x04);
	do {
		asm volatile(
			"ld %1, -0x08(%0)		\n" // Get the next return address
		: "=r"(return_address)
		: "r"(frame_pointer)
		:);
		printk("From %x\n", return_address-0x04);
		asm volatile(
			"ld %0, -0x10(%0)		\n" // Get next frame pointer
		: "+r"(frame_pointer)
		:
		:);
	} while (frame_pointer);
}

} // namespace io
