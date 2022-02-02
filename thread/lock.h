#include <stdint.h>

#ifndef THREAD_LOCK_H
#define THREAD_LOCK_H

namespace thread {

class Lock {
	public:
	bool try_lock();
	void lock();
	void unlock();

	private:
	uint64_t lock_primitive __attribute__((aligned (64))) = 0;
};

} // namespace thread

#endif
