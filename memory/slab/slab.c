/*
 * SLUB Object Cache Allocator
 */

#include <ABI/kernel/internal/kabi.h>

struct kmem_cache {
	const char *name;
	size_t size;
	size_t align;
	u32 flags;
	void (*ctor)(void *);
};

struct kmem_cache *kmem_cache_create(const char *name, size_t size, size_t align, u32 flags, void (*ctor)(void *))
{
	static struct kmem_cache cache_pool[32];
	static int cache_count = 0;

	if (cache_count >= 32)
		return (struct kmem_cache *)0;

	struct kmem_cache *c = &cache_pool[cache_count++];
	c->name = name;
	c->size = size;
	c->align = align;
	c->flags = flags;
	c->ctor = ctor;
	return c;
}
