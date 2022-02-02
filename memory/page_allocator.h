#ifndef MEMORY_PAGE_ALLOCATOR_H
#define MEMORY_PAGE_ALLOCATOR_H

#include <stdint.h>

namespace memory {

struct PageBlock {
	uint64_t start;
	uint64_t size;
};

int allocate_page_block(uint64_t target_size, PageBlock& block);

void free_page_block(PageBlock& block);

} // namespace memory

#endif
