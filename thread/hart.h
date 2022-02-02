#ifndef THREAD_HART_H
#define THREAD_HART_H

#include <stdint.h>

#include "config.h"

namespace thread { 

int64_t start_hart(void (*entry_func)(uint64_t, uint64_t), int hart_id, uint64_t arg, void* stack_top);

int64_t stop_hart();

} // namespace thread

#endif
