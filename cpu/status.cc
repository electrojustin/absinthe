#include "cpu/status.h"

#define CSR_SSTATUS 0x100

namespace cpu {

uint64_t get_status() {
	uint64_t ret;
	asm volatile(
		"csrr %0, %1		\n"
		: "=r"(ret)
		: "i"(CSR_SSTATUS)
		:);
	return ret;
}

void set_status(uint64_t new_status) {
	asm volatile(
		"csrw %1, %0		\n"
		:
		: "r"(new_status),
		  "i"(CSR_SSTATUS)
		:);
}

bool was_in_user_mode() {
	return !(get_status() & STATUS_SPP);
}

void set_sret_mode_user_mode() {
	uint64_t status = get_status();
	status &= ~STATUS_SPP;
	set_status(status);
}

void set_sret_mode_supervisor_mode() {
	uint64_t status = get_status();
	status |= STATUS_SPP;
	set_status(status);
}

bool interrupts_enabled() {
	return get_status() & STATUS_SIE;
}

void disable_interrupts() {
	uint64_t status = get_status();
	status &= ~STATUS_SIE;
	set_status(status);
}

void enable_interrupts() {
	uint64_t status = get_status();
	status |= STATUS_SIE;
	set_status(status);
}

} // namespace cpu
