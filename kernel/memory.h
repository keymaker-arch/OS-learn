#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

struct virtual_pool{
	struct bitmap vaddr_bitmap;
	uint32_t vaddr_start;
};

enum pool_flag{
	PF_KERNEL=1,
	PF_USER=2
};

#define PG_P 1		// present in physical memory
#define PG_RW_R 0	// read only
#define PG_RW_W 2	// write and read
#define PG_US_U 4	// access previlege:user
#define PG_US_S 0	// access previlege:supervisor

extern struct pool kernel_pool, user_pool;
void mem_init();
// apply for a memory block in kernel pool
void* apply_kernel_pages(uint32_t pg_count);


#endif