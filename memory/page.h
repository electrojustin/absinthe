#ifndef MEMORY_PAGE_H
#define MEMORY_PAGE_H

#include <stdint.h>

#include "config.h"
#include "thread/lock.h"

// Readable
#define PAGE_R 0b10
// Writeable
#define PAGE_W 0b100
// Executable
#define PAGE_X 0b1000
// User mode
#define PAGE_U 0b10000
// Global mapping (kernel region essentially)
#define PAGE_G 0b100000
// Read flag
#define PAGE_A 0b1000000
// Written flag
#define PAGE_D 0b10000000

namespace memory {

class MemoryRegion {
	public:
	MemoryRegion(uint64_t virtual_start, uint64_t virtual_end, uint16_t flags, bool managed_alloc=true);
	MemoryRegion(uint64_t virtual_start, uint64_t virtual_end, uint64_t physical_start, uint64_t physical_end, uint16_t flags, bool managed_alloc=false);
	~MemoryRegion();

	void allocate_pages();

	MemoryRegion* left;
	MemoryRegion* right;
	MemoryRegion* parent;
	uint64_t virtual_start;
	uint64_t virtual_end;
	uint64_t physical_start;
	uint64_t physical_end;

	// Indicates that we are automatically allocating pages (and thus responsible for freeing them).
	bool managed_alloc;

	uint16_t flags;
};

class PageTable {
	public:
	PageTable();
	~PageTable();

	int map_pages(uint64_t virtual_start, uint64_t virtual_end, uint16_t flags);
	// This variant assumes you've already allocated pages.
	int map_pages(uint64_t virtual_start, uint64_t virtual_end, uint64_t physical_start, uint64_t physical_end, uint16_t flags);
	int unmap_pages(uint64_t virtual_start, uint64_t virtual_end);
	void handle_page_fault(uint64_t virtual_addr, uint16_t flags);
	void use_page_table();
	static void disable_paging();

	private:
	// Tree of non-overlapping memory regions representing the memory map of the process (or kernel).
	MemoryRegion* root_memory_region;
	uint64_t* root_page_table;
	thread::Lock page_table_mutex;
	uint64_t cache_size = 0;

	void map_page(uint64_t virtual_addr, uint64_t physical_addr, uint16_t flags);
	uint64_t* get_page_table_entry(uint64_t virtual_addr);
	void insert_memory_region_helper(MemoryRegion* node, MemoryRegion* to_insert);
	void insert_memory_region(MemoryRegion* to_insert);
	void remove_memory_region(MemoryRegion* node);
	void find_contained_regions(uint64_t virtual_start, uint64_t virtual_end, lib::Queue<MemoryRegion*>& ret);
	void find_contained_regions(MemoryRegion* node, uint64_t virtual_start, uint64_t virtual_end, lib::Queue<MemoryRegion*>& ret);
	MemoryRegion* find_memory_region(uint64_t virtual_addr);
	MemoryRegion* find_memory_region(MemoryRegion* node, uint64_t virtual_addr);
	MemoryRegion* find_memory_region(uint64_t virtual_start, uint64_t virtual_end);
	MemoryRegion* find_memory_region(MemoryRegion* node, uint64_t virtual_start, uint64_t virtual_end);
};

} // namespace memory

#endif
