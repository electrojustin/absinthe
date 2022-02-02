#include "memory/heap.h"

#include "io/stdio.h"
#include "memory/page_allocator.h"
#include "thread/lock.h"

namespace memory {

namespace {

using io::printk;
using io::print_stack_trace;

thread::Lock heap_mutex;

struct __attribute__((packed)) FreeChunkHeader {
	FreeChunkHeader* next;
	FreeChunkHeader* prev;
	uint64_t size;
};

FreeChunkHeader* heap_start = nullptr;

void* kmalloc_internal(uint64_t size) {
	heap_mutex.lock();

	FreeChunkHeader* curr = heap_start;
	while(curr && curr->size < size) {
		curr = curr->next;
	}

	if (!curr) {
		heap_mutex.unlock();
		return nullptr;
	}

	if (curr->size - size > sizeof(FreeChunkHeader)) {
		FreeChunkHeader* new_chunk = curr + sizeof(FreeChunkHeader) + size;
		new_chunk->size = curr->size - size - sizeof(FreeChunkHeader);
		new_chunk->prev = curr->prev;
		new_chunk->next = curr->next;
		if (new_chunk->next) {
			new_chunk->next->prev = new_chunk;
		}
		if (new_chunk->prev) {
			new_chunk->prev->next = new_chunk;
		}

		curr->next = new_chunk;
		curr->size = size;
	}

	if (heap_start == curr) {
		heap_start = curr->next;
	}

	heap_mutex.unlock();

	return (void*)((uint64_t)curr + sizeof(FreeChunkHeader));
}

} // namespace


void defragment_heap() {
	heap_mutex.lock();

	if (!heap_start) {
		return;
	}

	FreeChunkHeader* curr = heap_start;
	FreeChunkHeader* streak_start = curr;
	FreeChunkHeader* streak_end = curr;

	while(curr) {
		if (curr + sizeof(FreeChunkHeader) + curr->size == curr->next) {
			streak_end = curr->next;
		} else {
			FreeChunkHeader new_chunk;
			new_chunk.next = streak_end->next;
			new_chunk.prev = streak_start->prev;
			new_chunk.size = (streak_end + streak_end->size + sizeof(FreeChunkHeader)) - (streak_start + sizeof(FreeChunkHeader));
			streak_start->next = new_chunk.next;
			streak_start->prev = new_chunk.prev;
			streak_start->size = new_chunk.size;
		}
		curr = curr->next;
	}

	heap_mutex.unlock();
}

int initialize_heap(uint64_t init_heap_size) {
	PageBlock heap_block;
	if (allocate_page_block(init_heap_size + sizeof(FreeChunkHeader), heap_block)) {
		printk("Cannot allocate heap of size %d.\n", init_heap_size);
		return -1;
	}

	heap_start = (FreeChunkHeader*)heap_block.start;
	heap_start->next = nullptr;
	heap_start->prev = nullptr;
	heap_start->size = heap_block.size - sizeof(FreeChunkHeader);

	return 0;
}

void* kmalloc(uint64_t size) {
	if (!heap_start) {
		printk("kmalloc: No heap!\n");
		print_stack_trace();
		return nullptr;
	}

	void* ret = kmalloc_internal(size);
	if (!ret) {
		defragment_heap();
		ret = kmalloc_internal(size);
	}	
	if (!ret) {
		printk("kmalloc: Out of memory!\n");
		print_stack_trace();
		return nullptr;
	}

	return ret;
}

void kfree(void* to_free) {
	heap_mutex.lock();

	FreeChunkHeader* new_free_chunk = (FreeChunkHeader*)((uint64_t)to_free - sizeof(FreeChunkHeader));

	if (!heap_start) {
		heap_start = new_free_chunk;
		heap_start->prev = nullptr;
		heap_start->next = nullptr;
		heap_mutex.unlock();
		return;
	}
	
	if (heap_start > new_free_chunk) {
		new_free_chunk->prev = nullptr;
		new_free_chunk->next = heap_start;
		heap_start->prev = new_free_chunk;
		heap_start = new_free_chunk;
		heap_mutex.unlock();
		return;
	}

	FreeChunkHeader* curr = heap_start;
	while(curr->next && curr->next < new_free_chunk) {
		curr = curr->next;
	}
	new_free_chunk->next = curr->next;
	new_free_chunk->prev = curr->prev;
	curr->next = new_free_chunk;
	if (new_free_chunk->next) {
		new_free_chunk->next->prev = new_free_chunk;
	}

	heap_mutex.unlock();
}

uint64_t calc_free_memory() {
	defragment_heap();

	heap_mutex.lock();

	uint64_t ret = 0;
	FreeChunkHeader* curr = heap_start;
	while(curr) {
		ret += curr->size;
		curr = curr->next;
	}

	heap_mutex.unlock();

	return ret;
}

} // namespace memory

void *operator new(size_t size) {
	return memory::kmalloc(size);
}

void *operator new[](size_t size) {
	return memory::kmalloc(size);
}

void operator delete(void* p) {
	memory::kfree(p);
}

void operator delete[](void* p) {
	memory::kfree(p);
}

void operator delete(void* p, size_t size) {
	memory::kfree(p);
}

void operator delete[](void* p, size_t size) {
	memory::kfree(p);
}
