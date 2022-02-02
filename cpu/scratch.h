#ifndef CPU_SCRATCH_H
#define CPU_SCRATCH_H

#include <stdint.h>

namespace cpu {

uint64_t get_scratch();

void set_scratch(uint64_t scratch);

} // namespace cpu

#endif
