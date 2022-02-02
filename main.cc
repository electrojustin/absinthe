#include "exec/executor.h"
#include "io/stdio.h"
#include "lib/queue.h"
#include "memory/heap.h"
#include "memory/page_allocator.h"
#include "thread/hart.h"
#include "thread/lock.h"

extern "C" {
uint8_t INITIAL_STACK[STACK_SIZE] __attribute__((aligned (64))) = {0};
}

void test3(void) {
	io::printk("test3\n");
	io::print_stack_trace();
}

void test2(void) {
	io::printk("test2\n");
	test3();
}

void test1(void) {
	io::printk("test\n");
	test2();
}

thread::Lock print_lock;

void foo() {
	while(1) {
		print_lock.lock();
		io::printk("foo\n");
		print_lock.unlock();
	}
}

void bar() {
	while(1) {
		print_lock.lock();
		io::printk("bar\n");
		print_lock.unlock();
	}
}

extern "C" void kernel_main(void) {
	io::puts("Hello, world!\n");
	io::printk("Testing, kernel_main loaded at %x\n", kernel_main);

	memory::initialize_heap(HEAP_SIZE);

	void* ptr1 = memory::kmalloc(300);
	void* ptr2 = memory::kmalloc(300);
	void* ptr3 = memory::kmalloc(300);
	io::printk("Free memory: %d\n", memory::calc_free_memory());
	memory::kfree(ptr1);
	memory::kfree(ptr2);
	io::printk("Free memory: %d\n", memory::calc_free_memory());

	test1();

	int* ptr4 = new int[64];
	io::printk("Free memory: %d\n", memory::calc_free_memory());

	exec::Executor executor;
	executor.exec(foo);
	executor.exec(bar);
	executor.start_threadpool();
//	thread::start_hart(foo, 0, 0);
//	thread::start_hart(bar, 1, 0);
}
