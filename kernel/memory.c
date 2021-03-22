#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "global.h"
#include "string.h"
#include "debug.h"

#define PG_SIZE 4096
#define MEM_BITMAP_BASE 0xc009a000
#define KERNEL_HEAP_START 0xc0100000

#define PDE_INDEX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_INDEX(addr) ((addr & 0x003ff000) >> 12)

struct pool
{
	// struct bitmap
	// {
	// 	uint32_t bitmap_bytes_len;
	// 	char* bits_ptr;		//bitmap pointer
	// };
	struct bitmap pool_bitmap;
	uint32_t phy_addr_start;
	uint32_t pool_size;		//in bytes
};


struct pool kernel_pool, user_pool;
struct virtual_pool kernel_vpool;
// struct virtual_pool{
// 	struct bitmap vaddr_bitmap;
// 	uint32_t vaddr_start;
// };

static void mem_pool_init(uint32_t all_mem){
	uint32_t pagitation_cost=PG_SIZE * 256;
	uint32_t used_mem=pagitation_cost + 0x100000;
	uint32_t free_mem=all_mem - used_mem;
	uint16_t all_free_pages = free_mem / PG_SIZE;

	// free pages
	uint16_t kernel_free_pages = all_free_pages/2;
	uint16_t user_free_pages = all_free_pages - kernel_free_pages;

	// 1 bit map to one physical page(4K), stored in bitmap by char
	uint32_t kernel_bitmap_len = kernel_free_pages / 8;
	uint32_t user_bitmap_len = user_free_pages / 8;

	// start addr of kernel pool and user pool(physical)
	uint32_t kernel_pool_start = used_mem;
	uint32_t user_pool_start = kernel_pool_start + kernel_free_pages * PG_SIZE;

	// assign kernel memory pool struct
	kernel_pool.pool_bitmap.bitmap_bytes_len = kernel_bitmap_len;
	kernel_pool.pool_bitmap.bits_ptr = (void*)MEM_BITMAP_BASE;
	kernel_pool.phy_addr_start = kernel_pool_start;
	kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
	// assign user memory pool struct
	user_pool.pool_bitmap.bitmap_bytes_len = user_bitmap_len;
	user_pool.pool_bitmap.bits_ptr = (void*)(MEM_BITMAP_BASE + kernel_bitmap_len);
	user_pool.phy_addr_start = user_pool_start;
	user_pool.pool_size = user_free_pages * PG_SIZE;

	// init bitmap
	bitmap_init(&kernel_pool.pool_bitmap);
	bitmap_init(&user_pool.pool_bitmap);

	put_string("kernel pool bitmap addr:");put_uint_hex((int)kernel_pool.pool_bitmap.bits_ptr);put_string("\n");
	put_string("kernel pool physical start addr:");put_uint_hex(kernel_pool.phy_addr_start);put_string("\n");
	put_string("user pool bitmap addr:");put_uint_hex((int)user_pool.pool_bitmap.bits_ptr);put_string("\n");
	put_string("user pool physical start addr:");put_uint_hex(user_pool.phy_addr_start);put_string("\n");

	// init kernel virtual memory pool
	kernel_vpool.vaddr_bitmap.bitmap_bytes_len = kernel_bitmap_len;
	kernel_vpool.vaddr_bitmap.bits_ptr = (void*)(MEM_BITMAP_BASE + kernel_bitmap_len + user_bitmap_len);
	kernel_vpool.vaddr_start = KERNEL_HEAP_START;
	bitmap_init(&kernel_vpool.vaddr_bitmap);
}


void mem_init(){
	uint32_t mem_total_bytes = (*(uint32_t*)MEM_TOTAL_STORE_ADDR);
	mem_pool_init(mem_total_bytes);
	put_string("memory init done\n\n\n");
}

// apply for PG_COUNT pages from PF virtual pool, return pointer to the area, return -1 if fail 
static void* apply_vaddr(enum pool_flag pf, uint32_t pg_count){
	int bit_index_start=-1;
	int count=0;
	int vaddr_start=0;
	if(pf == PF_KERNEL){
		bit_index_start = bitmap_scan(&kernel_vpool.vaddr_bitmap, pg_count);
		if(bit_index_start == -1){
			return NULL;
		}
		while(count<pg_count){
			bitmap_set_bit(&kernel_vpool.vaddr_bitmap, bit_index_start+count++, BITMAP_MASK);
		}
		vaddr_start = kernel_vpool.vaddr_start + bit_index_start * PG_SIZE;
	}else if(pf == PF_USER){
		//user virtual pool
	}
	return (void*)vaddr_start;
}


// get pointer(vritual) to vaddr's PTE 
uint32_t* get_PTE_ptr(uint32_t vaddr){
	uint32_t* ptr = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_INDEX(vaddr)*4);
	return ptr;
}


// get pointer(vritual) to vaddr's PDE 
uint32_t* get_PDE_ptr(uint32_t vaddr){
	uint32_t* ptr = (uint32_t*)(0xfffff000 + PDE_INDEX(vaddr)*4);
	return ptr;
}

// allocate one single physical page from MEM_POOL, return physical addr, return -1 if fail
static void* apply_page_physical(struct pool* mem_pool){
	int bit_index = bitmap_scan(&mem_pool->pool_bitmap, 1);
	if(bit_index == -1){
		return NULL;
	}
	bitmap_set_bit(&mem_pool->pool_bitmap, bit_index, BITMAP_MASK);
	uint32_t page_physical_addr = mem_pool->phy_addr_start + bit_index * PG_SIZE;
	return (void*)page_physical_addr;
}

// map virtual addr to physical addr
static void virtual_physical_addr_map(void* _vaddr, void* _page_physical_addr){
	uint32_t vaddr = (uint32_t)_vaddr;
	uint32_t page_physical_addr = (uint32_t)_page_physical_addr;
	uint32_t* PDE = get_PDE_ptr(vaddr);
	uint32_t* PTE = get_PTE_ptr(vaddr);

	if(*PDE & 0x00000001){		//page table existed, write physical addr to PTE
		ASSERT(!(*PTE & 0x00000001));	//page table entry should not exist
		if(!(*PTE & 0x00000001)){		//check again
			*PTE = page_physical_addr | PG_US_U | PG_RW_W | PG_P;
		}else{		//should not happen
			PANIC("PTE repeat!");
		}
	}else{		// page table not exists, write PDE first
		uint32_t page_table_physical_addr = (uint32_t)apply_page_physical(&kernel_pool);
		*PDE = page_table_physical_addr | PG_US_U | PG_RW_W | PG_P;

		// pte & 0xfffff000 points to the begin of page table([page_talbe_physical_addr])
		// init to 0 to prevent data in it becoming PTE
		memset((void*)((int)PTE & 0xfffff000), 0, PG_SIZE);

		ASSERT(!(*PTE & 0x00000001));
		*PTE = page_physical_addr | PG_US_U | PG_RW_W | PG_P;
	}
}


// apply PG_COUNT pages, add mapping in page table and return start addr(virtual) of the memory block
void* malloc_pages(enum pool_flag pf, uint32_t pg_count){
	ASSERT(pg_count > 0 && pg_count < 3000);
	void* vaddr_start = apply_vaddr(pf, pg_count);
	if(vaddr_start == NULL){
		return NULL;
	}

	struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
	uint32_t vaddr = (uint32_t)vaddr_start;
	void* page_physical_addr;
	while(pg_count-- > 0){
		page_physical_addr = apply_page_physical(mem_pool);
		ASSERT(page_physical_addr!=NULL);
		if(page_physical_addr == NULL){		//virtual addr is continuous, page addr(physical) isn't 

			return NULL;
		}
		virtual_physical_addr_map((void*)vaddr, page_physical_addr);
		vaddr += PG_SIZE;
	}
	return vaddr_start;
}


// apply for a memory block in kernel pool
void* apply_kernel_pages(uint32_t pg_count){
	void* vaddr = malloc_pages(PF_KERNEL, pg_count);
	if(vaddr != NULL){
		memset(vaddr, 0, pg_count * PG_SIZE);
	}
	return vaddr;
}















