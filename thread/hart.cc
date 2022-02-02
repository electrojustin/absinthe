#include <stdint.h>

#include "thread/hart.h"

namespace thread {

namespace {

extern "C" void* hart_entry;

asm volatile(
	"hart_entry:			\n"
	"add sp, zero, a1		\n"
	"ld s1, -0x40(sp)		\n"
	"ld a1, -0x80(sp)		\n"
	"la s0, 0			\n"
	"jalr s1			\n");

} // namespace

int64_t start_hart(void (*entry_func)(uint64_t, uint64_t), int hart_id, uint64_t arg, void* stack_top) {
	int64_t ret;
	*(uint64_t*)((uint8_t*)stack_top-128) = arg; 
	*(uint64_t*)((uint8_t*)stack_top-64) = (uint64_t)entry_func; 
	asm volatile(
		"add a0, zero, %1	\n" // Load hart id
		"add a1, zero, %2	\n" // Load start addr
		"add a2, zero, %3	\n" // Load argument
		"li a7, 0x48534D	\n" // EID: HSM
		"li a6, 0x00		\n" // FID: 0
		"ecall			\n"
		"add %0, zero, a0	\n"
		: "=r"(ret)
		: "r"(hart_id),
		  "r"(&hart_entry),
		  "r"(stack_top)
		: "a0", "a1", "a2", "a6", "a7");
	return ret;
}

int64_t stop_hart() {
	int64_t ret;
	asm volatile(
		"li a7, 0x48534D	\n" // EID: HSM
		"li a6, 0x01		\n" // FID: 0
		"ecall			\n"
		"add %0, zero, a0	\n"
		: "=r"(ret)
		:
		: "a6", "a7");
	return ret;
}

} // namespace thread
