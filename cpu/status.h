#ifndef CPU_STATUS_H
#define CPU_STATUS_H

#include <stdint.h>

#define STATUS_SPP 0b100000000
#define STATUS_SIE 0b10

namespace cpu {

uint64_t get_status();
void set_status(uint64_t new_status);

// Returns whether or not a trap was taken from user mode or machine/supervisor mode.
bool was_in_user_mode();

// Set the mode to return to after returning from a trap.
void set_sret_mode_user_mode();
void set_sret_mode_supervisor_mode();

bool interrupts_enabled();
void disable_interrupts();
void enable_interrupts();

} // namespace cpu

#endif
