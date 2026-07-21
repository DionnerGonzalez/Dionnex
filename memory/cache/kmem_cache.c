/*
 * Kernel Object Cache Primitives
 */

#include <ABI/kernel/internal/kabi.h>

void *kmem_cache_alloc(void *cachep, u32 flags)
{
	(void)cachep;
	(void)flags;
	return (void *)0;
}
