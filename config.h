#ifndef CONFIG_H
#define CONFIG_H

#define PAGE_SIZE 4096

// Kernel stack size 16k
#define STACK_SIZE 16384

// Kernel heap size 1M
#define HEAP_SIZE 1 << 20

// Physical memory map
#define KERNEL_START 0x80020000
#define KERNEL_END 0x80030000
#define FREE_MEMORY_START 0x80030000
#define FREE_MEMORY_END 0x88000000

// SMP
#define SMP_ENABLED
#define NUM_HART 4

#endif
