#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

#include <stddef.h>
#include <stdint.h>

namespace memory {

int initialize_heap(uint64_t init_heap_size);

void defragment_heap();

void* kmalloc(uint64_t size);

void kfree(void* to_free);

// This memory isn't guaranteed to be contiguous, so this is a crude health
// metric at best.
uint64_t calc_free_memory();

} // namespace memory

void *operator new(size_t size);

void *operator new[](size_t size);

void operator delete(void* p);

void operator delete[](void* p);

void operator delete(void* p, size_t size);

void operator delete[](void* p, size_t size);

#endif
