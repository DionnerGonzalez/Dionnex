/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Dynamic Heap Allocator
 * ============================================================================
 */
#include <mm/heap.h>
#include <mm/pmm.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <string.h>

extern uint32_t _kernel_end;

static heap_block_t *heap_start = NULL;
static uint32_t heap_end_addr = 0;

static void heap_coalesce(void) {
    heap_block_t *curr = heap_start;
    while (curr && curr->next) {
        if (curr->magic != HEAP_MAGIC) {
            kernel_panic("Heap corruption detected in coalesce");
        }
        if (curr->free && curr->next->free) {
            if (curr->next->magic != HEAP_MAGIC) {
                kernel_panic("Heap corruption detected in coalesce next");
            }
            curr->size += sizeof(heap_block_t) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void heap_init(uint32_t start, uint32_t size) {
    if (start == 0) {
        start = ALIGN_UP((uint32_t)&_kernel_end + 0x100000, PAGE_SIZE);
    }
    if (size < 0x100000) size = 0x100000; // Minimum 1MB initial heap

    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < pages; i++) {
        uint32_t p = pmm_alloc_page();
        if (p == 0) {
            kernel_panic("Heap: Out of memory allocating initial heap pages");
        }
        if (i == 0) start = p;
    }

    heap_start = (heap_block_t*)start;
    heap_start->magic = HEAP_MAGIC;
    heap_start->size = (pages * PAGE_SIZE) - sizeof(heap_block_t);
    heap_start->free = 1;
    heap_start->next = NULL;

    heap_end_addr = start + (pages * PAGE_SIZE);

    printk("Heap: Initialized %u KB at 0x%x\n", (pages * PAGE_SIZE) / 1024, start);
}

void *kmalloc(uint32_t size) {
    if (size == 0) return NULL;

    // Align size to 4 bytes
    uint32_t req_size = ALIGN_UP(size, 4);

    heap_block_t *curr = heap_start;
    heap_block_t *last = NULL;

    while (curr) {
        if (curr->magic != HEAP_MAGIC) {
            kernel_panic("Heap corruption detected");
        }
        if (curr->free && curr->size >= req_size) {
            if (curr->size >= req_size + sizeof(heap_block_t) + 16) {
                // Split block
                heap_block_t *next_block = (heap_block_t*)((uintptr_t)curr + sizeof(heap_block_t) + req_size);
                next_block->magic = HEAP_MAGIC;
                next_block->size = curr->size - req_size - sizeof(heap_block_t);
                next_block->free = 1;
                next_block->next = curr->next;

                curr->size = req_size;
                curr->next = next_block;
            }
            curr->free = 0;
            return (void*)((uintptr_t)curr + sizeof(heap_block_t));
        }
        last = curr;
        curr = curr->next;
    }

    // No free block found, expand heap from PMM
    uint32_t needed_bytes = req_size + sizeof(heap_block_t);
    uint32_t needed_pages = (needed_bytes + PAGE_SIZE - 1) / PAGE_SIZE;

    uint32_t new_page_addr = pmm_alloc_page();
    if (new_page_addr == 0) {
        return NULL; // Out of memory
    }
    for (uint32_t i = 1; i < needed_pages; i++) {
        pmm_alloc_page();
    }

    heap_block_t *new_block = (heap_block_t*)new_page_addr;
    new_block->magic = HEAP_MAGIC;
    new_block->size = (needed_pages * PAGE_SIZE) - sizeof(heap_block_t);
    new_block->free = 0;
    new_block->next = NULL;

    if (last) {
        last->next = new_block;
    } else {
        heap_start = new_block;
    }

    heap_end_addr = new_page_addr + (needed_pages * PAGE_SIZE);

    if (new_block->size >= req_size + sizeof(heap_block_t) + 16) {
        heap_block_t *rem = (heap_block_t*)((uintptr_t)new_block + sizeof(heap_block_t) + req_size);
        rem->magic = HEAP_MAGIC;
        rem->size = new_block->size - req_size - sizeof(heap_block_t);
        rem->free = 1;
        rem->next = NULL;

        new_block->size = req_size;
        new_block->next = rem;
    }

    return (void*)((uintptr_t)new_block + sizeof(heap_block_t));
}

void kfree(void *ptr) {
    if (!ptr) return;

    heap_block_t *block = (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        kernel_panic("Heap corruption detected");
    }

    block->free = 1;
    heap_coalesce();
}

void *krealloc(void *ptr, uint32_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_block_t *block = (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        kernel_panic("Heap corruption detected");
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
    uint32_t total = count * size;
    void *ptr = kmalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

uint32_t heap_get_free(void) {
    uint32_t free_bytes = 0;
    heap_block_t *curr = heap_start;
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
    heap_block_t *curr = heap_start;
    while (curr) {
        if (curr->magic == HEAP_MAGIC && !curr->free) {
            used_bytes += curr->size;
        }
        curr = curr->next;
    }
    return used_bytes;
}
