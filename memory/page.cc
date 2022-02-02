#include "memory/page.h"

#include "io/stdio.h"
#include "lib/memory.h"
#include "memory/page_allocator.h"
#include "memory/heap.h"

namespace memory {

namespace {

#define PAGING_39_BIT (8 << 60)

#define CSR_SATP 0x180

using lib::bzero;

}

MemoryRegion::MemoryRegion(uint64_t virtual_start, uint64_t virtual_end, bool managed_alloc=true) {
	this->virtual_start = virtual_start;
	this->virtual_end = virtual_end;
	this->managed_alloc = managed_alloc;
	this->physical_start = nullptr;
	this->physical_end = nullptr;
	this->left = nullptr;
	this->right = nullptr;
	this->parent = nullptr;
}

MemoryRegion::MemoryRegion(uint64_t virtual_start, uint64_t virtual_end, uint64_t physical_start, uint64_t physical_end) {
	if (virtual_end - virtual_start != physical_end - physical_start) {
		io::printk("Memory region %x-%x does not match size of physical region %x-%x", 
			   virtual_start,
			   virtual_end,
			   physical_start,
			   physical_end);
		io::print_stack_trace();
		return;
	}
	this->virtual_start = virtual_start;
	this->virtual_end = virtual_end;
	this->physical_start = physical_start;
	this->physical_end = physical_end;
	this->managed_alloc = managed_alloc;
	this->left = nullptr;
	this->right = nullptr;
	this->parent = nullptr;
}

MemoryRegion::~MemoryRegion() {
	if (managed_alloc) {
		PageBlock to_free;
		to_free.start = physical_start;
		to_free.size = physical_end - physical_start;
		free_page_block(to_free);
	}
}

void MemoryRegion::allocate_pages() {
	if (!managed_alloc) {
		io::printk("Cannot allocate pages of unmanaged memory region!\n");
		io::print_stack_trace();
		return;
	}

	PageBlock new_physical_page_block;
	size_t target_size = virtual_end - virtual_start;
	if (allocate_page_block(target_size, new_physical_page_block)) {
		io::printk("Cannot allocate page block for memory region %x-%x", virtual_start, virtual_end);
		io::print_stack_trace();
		return;
	}
	physical_start = new_physical_page_block.start;
	physical_end = physical_start + new_physical_page_block.size;
}

int PageTable::map_pages(uint64_t virtual_start, uint64_t virtual_end, uint16_t flags) {
	page_table_mutex.lock();

	MemoryRegion* to_insert = new MemoryRegion(virtual_start, virtual_end, flags);
	insert_memory_region(to_insert);

	page_table_mutex.unlock();
}

int PageTable::map_pages(uint64_t virtual_start, uint64_t virtual_end, uint64_t physical_start, uint64_t physical_end, uint16_t flags) {
	page_table_mutex.lock();

	MemoryRegion* to_insert = new MemoryRegion(virtual_start, virtual_end, physical_start, physical_end, flags);
	insert_memory_region(to_insert);

	page_table_mutex.unlock();
}

int PageTable::unmap_pages(uint64_t virtual_start, uint64_t virtual_end) {
	page_table_mutex.lock();

	MemoryRegion* to_delete = new MemoryRegion(virtual_start, virtual_end, 0);
	insert_memory_region(to_delete);
	remove_memory_region(to_delete);

	page_table_mutex.unlock();
}

void PageTable::handle_page_fault(uint64_t virtual_addr, uint16_t flags) {
	disable_paging();

	page_table_mutex.lock();

	MemoryRegion* region = find_memory_region(virtual_addr);
	if (region) {
		if (region->flags & flags == flags) {
			if (!get_page_table_entry(virtual_addr)) {
				if (region->managed_alloc && !region->physical_start) {
					if (!region->physical_start) {
						region->allocate_pages();
					}
				}

				uint64_t physical_addr = region->physical_start + (virtual_addr - region->virtual_start);
				map_page(virtual_addr, physical_addr, region->flags);

				if (region->managed_alloc) {
					bzero((uint8_t*)physical_addr, PAGE_SIZE);
				}

				page_table_mutex.unlock();
				return;
			}
		}
	}

	io::printk("Unexpected page fault!\n");
	io::print_stack_trace();

	page_table_mutex.unlock();

	use_page_table();
}

void PageTable::use_page_table() {
	uint64_t satp_val = (((uint64_t)root_page_table)/PAGE_SIZE) | PAGING_39_BIT;
	asm volatile(
		"csrw %1, %0		\n"
		: "+r"(satp_val)
		: "i"(CSR_SATP)
		:);
}

static void PageTable::disable_paging() {
	asm volatile(
		"csrw %0, zero		\n"
		:
		: "i"(CSR_SATP)
		:);
}

void map_page(uint64_t virtual_addr, uint64_t physical_addr, uint16_t flags) {
	PageBlock new_table;
	virtual_addr >>= 12;
	if (!root_page_table[virtual_addr & 0x1FF]) {
		if (allocate_page_block(PAGE_SIZE, new_table)) {
			io::printk("Error allocating page table entry!\n");
			io::print_stack_trace();
		}
		root_page_table[virtual_addr & 0x1FF] = new_table.start << 10;
		bzero((uint8_t*)new_table.start, PAGE_SIZE);
	}

	uint64_t* entry = root_page_table[virtual_addr & 0x1FF];
	virtual_addr >>= 9;
	if (!entry[virtual_addr & 0x1FF]) {
		if (allocate_page_block(PAGE_SIZE, new_table)) {
			io::printk("Error allocating page table entry!\n");
			io::print_stack_trace();
		}
		entry[virtual_addr & 0x1FF] = new_table.start << 10;
		bzero((uint8_t*)new_table.start, PAGE_SIZE);
	}

	entry = entry[virtual_addr & 0x1FF];
	virtual_addr >>= 9;
	entry[virtual_addr & 0x1FF] = (physical_addr << 10) | flags | 1;
}

uint64_t* get_page_table_entry(uint64_t virtual_addr) {
	virtual_addr >>= 12;
	uint64_t* entry = root_page_table;
	int i = 0;
	while(i < 2 &&
	      entry[virtual_addr & 0x1FF] && 
	      !(entry[virtual_addr & 0x1FF] & (PAGE_R | PAGE_W | PAGE_X))) {
		entry = (uint64_t*)((entry[virtual_addr & 0x1FF] >> 10) & 0x7FFFFFFFF);
		virtual_addr >>= 9;
		i++;
	}

	return entry + (virtual_addr & 0x1FF);
}

void PageTable::insert_memory_region_helper(MemoryRegion* node, MemoryRegion* to_insert) {
	if (!root_memory_region) {
		root_memory_region = node;
		return;
	}

	if (node->virtual_start <= to_insert->virtual_start) {
		if (node->left) {
			insert_memory_region_helper(node->left, to_insert);
		} else {
			node->left = to_insert;
			to_insert->parent = node;
		}
	} else {
		if (node->right) {
			insert_memory_region_helper(node->right, to_insert);
		} else {
			node->right = to_insert;
			to_insert->parent = node;
		}
	}
}

void PageTable::insert_memory_region(MemoryRegion* to_insert) {
	if (root_memory_region) {
		// Delete all regions that are completely overlapped.
		lib::Queue<MemoryRegion*> contained_regions;
		find_contained_regions(to_insert->virtual_start, to_insert->virtual_end, contained_regions);
		MemoryRegion* to_remove = nullptr;
		while(contained_regions.dequeue(to_remove)) {
			remove_memory_region(to_remove);
		}

		// Find region that is split on the lower bound.
		MemoryRegion* to_split = find_memory_region(to_insert->virtual_start);
		if (to_split) {
			MemoryRegion discard(to_insert->virtual_start, to_split->virtual_end, to_split->managed_alloc);
			to_split->virtual_end = to_insert->virtual_start;
			to_split->physical_end = to_split->physical_start + (to_split->virtual_end - to_split->virtual_start);
			if (to_split->virtual_end <= to_insert->virtual_end) {
				// Find a region that is split on the upper bound.
				to_split = find_memory_region(to_insert->virtual_end);
				if (to_split) {
					MemoryRegion discard(to_split->virtual_start, to_insert->virtual_end, to_split->managed_alloc);
					to_split->virtual_start = to_insert->virtual_end;
					to_split->physical_end = to_split->physical_start + (to_split->virtual_end - to_split->virtual_start);
				}
			} else {
				// We're actually splitting the same region twice, handle this specially.
				discard.virtual_end = to_insert->virtual_end;
				MemoryRegion* new_right_region = new MemoryRegion(to_insert->virtual_end, to_split->virtual_end,
										  to_split->physical_end - (to_split->virtual_end - to_insert->virtual_end),
										  to_split->physical_end,
										  to_split->flags,
										  to_split->managed_alloc);
				insert_memory_region_helper(root_memory_region, new_right_region);
			}
		}
	
		insert_memory_region_helper(root_memory_region, to_insert);
	} else {
		root_memory_region = to_insert;
	}
}

void remove_memory_region(MemoryRegion* node) {
	MemoryRegion* parent = node->parent;
	MemoryRegion* primary_subtree = nullptr;
	MemoryRegion* secondary_subtree = nullptr;
	if (node->left) {
		primary_subtree = node->left;
		secondary_subtree = node->right;
	} else {
		primary_subtree = node->right;
		secondary_subtree = node->left;
	}
	if (parent) {
		if (node == parent->left) {
			parent->left = primary_subtree;
		} else {
			parent->right = primary_subtree;
		}
	} else {
		root_memory_region = primary_subtree;
	}
	if (secondary_subtree) {
		insert_memory_region_helper(parent, secondary_subtree);
	}
	delete node;
}

void find_contained_regions(uint64_t virtual_start, uint64_t virtual_end, lib::Queue<MemoryRegion*>& ret) {
	find_contained_regions(root_memory_region, virtual_start, virtual_end, ret);
}

void find_contained_regions(MemoryRegion* node, uint64_t virtual_start, uint64_t virtual_end, lib::Queue<MemoryRegion*>& ret) {
	if (!node) {
		return;
	} else if (node->virtual_start >= virtual_start && node->virtual_end <= virtual_end) {
		find_contained_regions(node->left, virtual_start, virtual_end, ret);
		ret->enqueue(node);
		find_contained_regions(node->right, virtual_start, virtual_end, ret);
	} else if (virtual_end >= node->virtual_end) {
		find_contained_regions(node->left, virtual_start, virtual_end, ret);
	} else {
		find_contained_regions(node->right, virtual_start, virtual_end, ret);
	}
}

MemoryRegion* PageTable::find_memory_region(uint64_t virtual_addr) {
	return find_memory_region(root_memory_region, virtual_addr);
}

MemoryRegion* PageTable::find_memory_region(MemoryRegion* node, uint64_t virtual_addr) {
	if (!node) {
		return nullptr;
	} else if (node->virtual_start <= virtual_addr && node->virtual_end >= virtual_addr) {
		return node;
	} else if (node->virtual_end < virtual_addr) {
		return find_memory_region(node->right, virtual_addr);
	} else {
		return find_memory_region(node->left, virtual_addr);
	}
}

MemoryRegion* PageTable::find_memory_region(MemoryRegion* node, uint64_t virtual_start, uint64_t virtual_end) {
	if (!node) {
		return nullptr;
	} else if (virtual_start == node->virtual_start && virtual_end == node->virtual_end) {
		return node;
	} else if (virtual_start <= node->virtual_start) {
		return find_memory_region(node->left, virtual_start, virtual_end);
	} else {
		return find_memory_region(node->right, virtual_start, virtual_end);
	}
}

MemoryRegion* PageTable::find_memory_region(uint64_t virtual_start, uint64_t virtual_end) {
	return find_memory_region(root_memory_region, virtual_start, virtual_end);
}

} // namespace memory
