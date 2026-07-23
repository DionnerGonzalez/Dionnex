/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Heap Manager Header
 * ============================================================================
 */
#ifndef _DIONNEX_HEAP_H_
#define _DIONNEX_HEAP_H_

#include <types.h>

#define HEAP_MAGIC 0xDEADBEEF

struct heap_block {
    uint32_t magic;
    uint32_t size;
    uint8_t free;
    struct heap_block *next;
} __attribute__((packed));

typedef struct heap_block heap_block_t;

void heap_init(uint32_t start, uint32_t size);
void *kmalloc(uint32_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, uint32_t new_size);
void *kcalloc(uint32_t count, uint32_t size);
uint32_t heap_get_free(void);
uint32_t heap_get_used(void);

#endif /* _DIONNEX_HEAP_H_ */
