#include "memory/page_allocator.h"

#include "config.h"
#include "io/stdio.h"
#include "thread/lock.h"

namespace memory {

namespace {

thread::Lock page_alloc_mutex;

uint64_t lowest_free_index = 0;

uint8_t allocation_bitmap[(FREE_MEMORY_END - FREE_MEMORY_START) / PAGE_SIZE / 8];

char check_allocation(uint64_t index) {
	return (allocation_bitmap[index/8] >> (index % 8)) & 0x01;
}

void set_allocation(uint64_t index) {
	allocation_bitmap[index/8] |= (0x01 << (index % 8));
}

void clear_allocation(uint64_t index) {
	allocation_bitmap[index/8] &= (0xFE << (index % 8));
}

} // namespace

int allocate_page_block(uint64_t target_size, PageBlock& block) {
	page_alloc_mutex.lock();

	uint64_t actual_size = target_size;
	if (actual_size % PAGE_SIZE) {
		actual_size = ((actual_size / PAGE_SIZE) + 1) * PAGE_SIZE;
	}

	uint64_t bitmap_size = actual_size / PAGE_SIZE;

	uint64_t current_streak = 0;
	uint64_t index = lowest_free_index;
	uint64_t current_streak_start = 0;
	for (index; index < sizeof(allocation_bitmap) * 8; index++) {
		if (check_allocation(index)) {
			current_streak = 0;
		} else {
			if (!current_streak) {
				current_streak_start = index;
			}
			current_streak++;
		}

		if (current_streak == bitmap_size) {
			break;
		}
	}

	if (index == sizeof(allocation_bitmap)*8) {
		io::printk("allocate_page_block: Not enough contiguous pages!\n");
		io::print_stack_trace();
		return -1;
	}

	for (index = current_streak_start; index < current_streak_start + bitmap_size; index++) {
		set_allocation(index);

		if (current_streak_start == lowest_free_index) {
			lowest_free_index = current_streak_start + bitmap_size;
		}
	}

	block.start = current_streak_start * PAGE_SIZE + FREE_MEMORY_START;
	block.size = actual_size;

	page_alloc_mutex.unlock();

	return 0;
}

void free_page_block(PageBlock& block) {
	page_alloc_mutex.lock();

	if (lowest_free_index < (block.start - FREE_MEMORY_START) / PAGE_SIZE) {
		lowest_free_index = (block.start - FREE_MEMORY_START) / PAGE_SIZE;
	}
	for (uint64_t index = (block.start - FREE_MEMORY_START) / PAGE_SIZE; index < block.size / PAGE_SIZE; index++) {
		clear_allocation(index);	
	}

	page_alloc_mutex.unlock();
}

} // namespace memory
