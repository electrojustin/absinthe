#include "thread/lock.h"

namespace thread {

bool Lock::try_lock() {
	int ret = 0;
	asm volatile(
		"add a0, zero, %1		\n" // Load lock primitive addr
		"li t0, 1			\n" // Load locked value
		"amoswap.w.aq t0, t0, (a0)	\n" // Attempt to acquire lock
		"bnez t0, lock_failure		\n" // Check if we succeeded
		"li %0, 1			\n" // Return true
		"j lock_acquire_end		\n" // Jump passed fail condition
		"lock_failure:			\n"
		"li %0, 0			\n" // Return false
		"lock_acquire_end:		\n"
		: "=r"(ret)			    // %0
		: "r"(&lock_primitive) 		    // %1
		);
	return ret == 1;
}

void Lock::lock() {
	while(!try_lock());
}

void Lock::unlock() {
	asm volatile(
		"add a0, zero, %0		\n" // Load lock primitive addr
		"li t0, 0			\n" // Load unlocked value
		"amoswap.w.rl t0, t0, (a0)	\n"
		"fence rw,w			\n" // This isn't strictly
						    // necessary, but virtually
						    // every use case for a
						    // lock will want to fence
						    // too.
		:
		: "r"(&lock_primitive)		    // %0
		);
}

} // namespace thread
