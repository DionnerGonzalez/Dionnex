/*
 * Direct Memory Access (DMA) Coherent Buffer Allocator
 */

#include <ABI/kernel/internal/kabi.h>

void *dma_alloc_coherent(size_t size, phys_addr_t *dma_handle)
{
	if (!dma_handle || size == 0)
		return (void *)0;

	*dma_handle = 0x80000000; /* Physical DMA bus address */
	return (void *)0xFFFF880080000000ULL; /* Kernel virtual address */
}
