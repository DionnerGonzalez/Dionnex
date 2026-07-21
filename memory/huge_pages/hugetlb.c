/*
 * HugeTLB / 2MB & 1GB Large Frame Allocator
 */

#include <ABI/kernel/internal/kabi.h>

#define HPAGE_SIZE_2M (2ULL * 1024 * 1024)
#define HPAGE_SIZE_1G (1ULL * 1024 * 1024 * 1024)

phys_addr_t alloc_huge_page(size_t page_size)
{
	if (page_size != HPAGE_SIZE_2M && page_size != HPAGE_SIZE_1G)
		return 0;

	return 0x200000;
}
