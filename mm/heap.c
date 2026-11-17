/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Dynamic Heap Allocator
 * ============================================================================
 */
#include <mm/heap.h>
#include <mm/pmm.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <string.h>

#define HEAP_ALIGN 8U

static heap_block_t *heap_head = NULL;
static heap_block_t *heap_tail = NULL;

static void heap_coalesce_block(heap_block_t *block) {
    if (!block || !block->free) return;

    /* Coalesce with next ONLY if physically contiguous in memory */
    while (block->next && block->next->free) {
        heap_block_t *nxt = block->next;
        if (nxt->magic != HEAP_MAGIC) {
            kernel_panic("Heap corruption detected in coalesce (next)");
        }
        uintptr_t expected_next = (uintptr_t)block + sizeof(heap_block_t) + block->size;
        if ((uintptr_t)nxt != expected_next) {
            break; /* Non-contiguous heap segment boundary */
        }
        block->size += sizeof(heap_block_t) + nxt->size;
        block->next = nxt->next;
        if (block->next) {
            block->next->prev = block;
        } else {
            heap_tail = block;
        }
    }

    /* Coalesce with prev ONLY if physically contiguous in memory */
    while (block->prev && block->prev->free) {
        heap_block_t *prv = block->prev;
        if (prv->magic != HEAP_MAGIC) {
            kernel_panic("Heap corruption detected in coalesce (prev)");
        }
        uintptr_t expected_curr = (uintptr_t)prv + sizeof(heap_block_t) + prv->size;
        if ((uintptr_t)block != expected_curr) {
            break; /* Non-contiguous heap segment boundary */
        }
        prv->size += sizeof(heap_block_t) + block->size;
        prv->next = block->next;
        if (prv->next) {
            prv->next->prev = prv;
        } else {
            heap_tail = prv;
        }
        block = prv;
    }
}

static heap_block_t *heap_grow(uint32_t min_payload_size) {
    uint32_t needed_bytes = min_payload_size + sizeof(heap_block_t);
    uint32_t needed_pages = (needed_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    if (needed_pages < 4) needed_pages = 4; /* Allocate at least 16KB per segment */

    uint32_t seg_phys = pmm_alloc_low_pages(needed_pages);
    if (seg_phys == 0) {
        /* Fallback to exact page count if 16KB contiguous unavailable */
        needed_pages = (needed_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
        seg_phys = pmm_alloc_low_pages(needed_pages);
        if (seg_phys == 0) return NULL;
    }

    heap_block_t *seg = (heap_block_t*)seg_phys;
    seg->magic = HEAP_MAGIC;
    seg->size = (needed_pages * PAGE_SIZE) - sizeof(heap_block_t);
    seg->free = 1;
    seg->reserved[0] = seg->reserved[1] = seg->reserved[2] = 0;
    seg->prev = heap_tail;
    seg->next = NULL;

    if (heap_tail) {
        heap_tail->next = seg;
    } else {
        heap_head = seg;
    }
    heap_tail = seg;

    heap_coalesce_block(seg);
    return seg;
}

void heap_init(uint32_t start, uint32_t size) {
    (void)start;
    if (size < 0x100000U) size = 0x100000U; /* Minimum 1MB initial heap */

    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t heap_phys = pmm_alloc_low_pages(pages);
    if (heap_phys == 0) {
        pages = 256; /* Fallback to 1MB */
        heap_phys = pmm_alloc_low_pages(pages);
        if (heap_phys == 0) {
            kernel_panic("Heap: Out of contiguous low memory for initial heap");
        }
    }

    heap_head = (heap_block_t*)heap_phys;
    heap_head->magic = HEAP_MAGIC;
    heap_head->size = (pages * PAGE_SIZE) - sizeof(heap_block_t);
    heap_head->free = 1;
    heap_head->reserved[0] = heap_head->reserved[1] = heap_head->reserved[2] = 0;
    heap_head->next = NULL;
    heap_head->prev = NULL;
    heap_tail = heap_head;

    printk("Heap: Initialized %u KB contiguous kernel heap at 0x%x\n",
           (pages * PAGE_SIZE) / 1024, heap_phys);
}

void *kmalloc(uint32_t size) {
    if (size == 0 || !heap_head) return NULL;

    uint32_t req_size = ALIGN_UP(size, HEAP_ALIGN);
    if (req_size < size) return NULL; /* Overflow */

    heap_block_t *curr = heap_head;
    while (curr) {
        if (curr->magic != HEAP_MAGIC) {
            kernel_panic("Heap corruption detected in kmalloc");
        }
        if (curr->free && curr->size >= req_size) {
            if (curr->size >= req_size + sizeof(heap_block_t) + 16U) {
                heap_block_t *rem = (heap_block_t*)((uintptr_t)curr + sizeof(heap_block_t) + req_size);
                rem->magic = HEAP_MAGIC;
                rem->size = curr->size - req_size - sizeof(heap_block_t);
                rem->free = 1;
                rem->reserved[0] = rem->reserved[1] = rem->reserved[2] = 0;
                rem->prev = curr;
                rem->next = curr->next;

                if (curr->next) {
                    curr->next->prev = rem;
                } else {
                    heap_tail = rem;
                }
                curr->size = req_size;
                curr->next = rem;
            }
            curr->free = 0;
            return (void*)((uintptr_t)curr + sizeof(heap_block_t));
        }
        curr = curr->next;
    }

    /* Expand heap with new contiguous segment */
    heap_block_t *new_seg = heap_grow(req_size);
    if (!new_seg) return NULL;

    /* Retry allocation on expanded segment */
    curr = heap_head;
    while (curr) {
        if (curr->free && curr->size >= req_size) {
            if (curr->size >= req_size + sizeof(heap_block_t) + 16U) {
                heap_block_t *rem = (heap_block_t*)((uintptr_t)curr + sizeof(heap_block_t) + req_size);
                rem->magic = HEAP_MAGIC;
                rem->size = curr->size - req_size - sizeof(heap_block_t);
                rem->free = 1;
                rem->reserved[0] = rem->reserved[1] = rem->reserved[2] = 0;
                rem->prev = curr;
                rem->next = curr->next;

                if (curr->next) {
                    curr->next->prev = rem;
                } else {
                    heap_tail = rem;
                }
                curr->size = req_size;
                curr->next = rem;
            }
            curr->free = 0;
            return (void*)((uintptr_t)curr + sizeof(heap_block_t));
        }
        curr = curr->next;
    }

    return NULL;
}

void *kmalloc_aligned(uint32_t size, uint32_t align) {
    if (size == 0) return NULL;
    if (align < HEAP_ALIGN) align = HEAP_ALIGN;
    /* Power-of-two check */
    if ((align & (align - 1U)) != 0) return NULL;

    uint32_t total = size + align + sizeof(uintptr_t);
    if (total < size) return NULL;

    void *raw = kmalloc(total);
    if (!raw) return NULL;

    uintptr_t start = (uintptr_t)raw + sizeof(uintptr_t);
    uintptr_t aligned = (start + align - 1U) & ~(uintptr_t)(align - 1U);
    ((uintptr_t*)aligned)[-1] = (uintptr_t)raw;
    return (void*)aligned;
}

void kfree(void *ptr) {
    if (!ptr) return;

    /* Check if ptr is a direct heap block or aligned wrapper? Caller uses kfree_aligned or standard kfree */
    heap_block_t *block = (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        /* Could be an aligned pointer whose raw pointer sits right below */
        uintptr_t maybe_raw = ((uintptr_t*)ptr)[-1];
        if (maybe_raw > 0x100000U && maybe_raw < (uintptr_t)ptr) {
            heap_block_t *ablock = (heap_block_t*)(maybe_raw - sizeof(heap_block_t));
            if (ablock->magic == HEAP_MAGIC) {
                block = ablock;
            } else {
                kernel_panic("Heap corruption detected in kfree");
            }
        } else {
            kernel_panic("Heap corruption detected in kfree");
        }
    }

    if (block->free) {
        pr_warn("Heap: Double free detected at 0x%x\n", (uint32_t)ptr);
        return;
    }

    block->free = 1;
    heap_coalesce_block(block);
}

void *krealloc(void *ptr, uint32_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_block_t *block = (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        kernel_panic("Heap corruption detected in krealloc");
    }

    if (block->size >= new_size) {
        return ptr;
    }

    void *new_ptr = kmalloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }
    return new_ptr;
}

void *kcalloc(uint32_t count, uint32_t size) {
    if (count == 0 || size == 0) return kmalloc(1);
    if (count > 0xFFFFFFFFU / size) return NULL; /* Integer overflow guard */

    uint32_t total = count * size;
    void *ptr = kmalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

uint32_t heap_get_free(void) {
    uint32_t free_bytes = 0;
    heap_block_t *curr = heap_head;
    while (curr) {
        if (curr->magic == HEAP_MAGIC && curr->free) {
            free_bytes += curr->size;
        }
        curr = curr->next;
    }
    return free_bytes;
}

uint32_t heap_get_used(void) {
    uint32_t used_bytes = 0;
    heap_block_t *curr = heap_head;
    while (curr) {
        if (curr->magic == HEAP_MAGIC && !curr->free) {
            used_bytes += curr->size;
        }
        curr = curr->next;
    }
    return used_bytes;
}

int heap_validate(void) {
    heap_block_t *curr = heap_head;
    heap_block_t *prev = NULL;
    uint32_t count = 0;
    while (curr) {
        if (curr->magic != HEAP_MAGIC) return -1;
        if (curr->prev != prev) return -2;
        prev = curr;
        curr = curr->next;
        if (++count > 1000000U) return -3; /* Cycle guard */
    }
    return 0;
}
