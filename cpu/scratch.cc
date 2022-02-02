#include "cpu/scratch.h"

#define CSR_SSCRATCH 0x140

namespace cpu {

uint64_t get_scratch() {
	uint64_t ret;
	asm volatile(
		"csrr %0, %1		\n"
		: "=r"(ret)
		: "i"(CSR_SSCRATCH)
		:);
	return ret;
}

void set_scratch(uint64_t scratch) {
	asm volatile(
		"csrw %1, %0		\n"
		:
		: "r"(scratch),
		  "i"(CSR_SSCRATCH)
		:);
}

} // namespace cpu
