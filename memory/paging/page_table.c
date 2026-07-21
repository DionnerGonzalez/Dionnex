/*
 * Generic Architecture Paging Operations
 */

#include <ABI/kernel/internal/kabi.h>

void map_kernel_page(virt_addr_t vaddr, phys_addr_t paddr, u64 flags)
{
	(void)vaddr;
	(void)paddr;
	(void)flags;
}
