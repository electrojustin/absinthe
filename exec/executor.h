#ifndef EXEC_EXECUTOR_H
#define EXEC_EXECUTOR_H

#include <functional>

#include "lib/queue.h"
#include "config.h"

namespace exec {

class Executor;

struct ExecContext {
	void* kernel_stack_top;
	int hart_id;
	Executor* executor;
	//TODO: Page table here
};

class Executor {
	public:
	Executor();
	~Executor();
	void exec(std::function<void()> to_run);
	void work(int hart_id);
	void shutdown();
	void drain();
	void start_threadpool(); // Does not return

	private:
	bool is_draining = false;
	bool is_running = true;
	lib::Queue<std::function<void()>> work_queue;
	ExecContext default_contexts[NUM_HART];
	uint8_t** hart_stacks;
};

void worker_entry(uint64_t hart_id, uint64_t exec_context_ptr);

} // namespace exec

#endif
