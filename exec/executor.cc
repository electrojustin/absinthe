#include "exec/executor.h"
#include "cpu/scratch.h"
#include "thread/hart.h"
#include "io/stdio.h"
#include "lib/memory.h"
#include "memory/heap.h"
#include "memory/page_allocator.h"

namespace std {

// Hack to get std::function to work without libc++
void __throw_bad_function_call() {
	io::printk("Bad function call!");
	io::print_stack_trace();
}

} // namespace std

namespace exec {

Executor::Executor() {
	hart_stacks = (uint8_t**)memory::kmalloc(NUM_HART*sizeof(uint8_t*));
	for (int hart_id = 0; hart_id < NUM_HART; hart_id++) {
		memory::PageBlock block;
		if (memory::allocate_page_block(STACK_SIZE, block)) {
			io::printk("Error allocating HART stacks!\n");
			io::print_stack_trace();
		}
		hart_stacks[hart_id] = (uint8_t*)block.start;
	}

	for (int hart_id = 0; hart_id < NUM_HART; hart_id++) {
		default_contexts[hart_id].hart_id = hart_id;
		default_contexts[hart_id].kernel_stack_top = &(hart_stacks[hart_id][STACK_SIZE]);
		default_contexts[hart_id].executor = this;
	}
}

Executor::~Executor() {
	for (int hart_id = 0; hart_id < NUM_HART; hart_id++) {
		memory::PageBlock block;
		block.start = (uint64_t)hart_stacks[hart_id];
		block.size = STACK_SIZE;
		memory::free_page_block(block);
	}
	memory::kfree(hart_stacks);
}

void Executor::exec(std::function<void()> to_run) {
	if (!is_draining) {
		work_queue.enqueue(to_run);
	}
}

void Executor::work(int hart_id) {
	while(is_running) {
		std::function<void()> task;
		if (work_queue.dequeue(task)) {
			task();
		} else if (is_draining) {
			is_running = false;
		}
	}
}

void Executor::shutdown() {
	is_running = false;
}

void Executor::drain() {
	is_draining = true;
}

void Executor::start_threadpool() {
	int curr_hart_id = 0;
	for (int hart_id = 0; hart_id < NUM_HART; hart_id++) {
		if (thread::start_hart(worker_entry, 
				       hart_id, 
				       (uint64_t)&(default_contexts[hart_id]), 
				       &(hart_stacks[hart_id][STACK_SIZE]))) {
			curr_hart_id = hart_id;
		}
	}
	worker_entry(curr_hart_id, (uint64_t)&(default_contexts[curr_hart_id]));
}

void worker_entry(uint64_t hart_id, uint64_t exec_context_ptr) {
	ExecContext* context = (ExecContext*)exec_context_ptr;
	Executor* executor = context->executor;
	cpu::set_scratch(0);
	executor->work(hart_id);
	thread::stop_hart();
}

} // namespace exec
