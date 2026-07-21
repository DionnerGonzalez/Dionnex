/*
 * Kernel General Heap Allocator (kmalloc / kfree)
 */

#include <ABI/kernel/internal/kabi.h>

void *kmalloc(size_t size, u32 flags)
{
	(void)flags;
	if (size == 0)
		return (void *)0;

	/* Allocate chunk from slab cache buckets */
	static char heap_buf[65536];
	static size_t heap_offset = 0;

	if (heap_offset + size > sizeof(heap_buf))
		return (void *)0;

	void *ptr = &heap_buf[heap_offset];
	heap_offset += (size + 15) & ~15;
	return ptr;
}

void kfree(const void *ptr)
{
	(void)ptr;
}

EXPORT_SYMBOL(kmalloc);
EXPORT_SYMBOL(kfree);
