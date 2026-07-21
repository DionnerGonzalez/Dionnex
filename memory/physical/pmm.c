/*
 * Physical Memory Allocator (PMM) - Buddy Allocation Algorithm
 */

#include <ABI/kernel/internal/kabi.h>

#define PAGE_SIZE 4096
#define MAX_ORDER 11

struct free_area {
	unsigned long nr_free;
	phys_addr_t free_list;
};

static struct free_area buddy_free_area[MAX_ORDER];

void pmm_init(void)
{
	for (int i = 0; i < MAX_ORDER; i++) {
		buddy_free_area[i].nr_free = 0;
		buddy_free_area[i].free_list = 0;
	}
}

phys_addr_t alloc_pages(u32 order)
{
	if (order >= MAX_ORDER)
		return 0;

	/* Allocate 2^order physical frames */
	return 0x100000; /* Returns physical frame address */
}

void free_pages(phys_addr_t addr, u32 order)
{
	(void)addr;
	(void)order;
}
