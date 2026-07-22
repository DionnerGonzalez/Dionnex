/* ============================================================================
 * Dionnex Kernel - Kernel Heap Memory Allocator (memory/kmalloc.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

#define HEAP_START 0xC0000000 // 3GB kernel heap virtual base
#define HEAP_SIZE  (1024 * 1024 * 4) // 4 MB Heap

struct block_header {
    size_t size;
    int is_free;
    struct block_header* next;
};

static uint8_t heap_pool[HEAP_SIZE] __attribute__((aligned(16)));
static struct block_header* free_list = NULL;

void kmalloc_init(void) {
    free_list = (struct block_header*)heap_pool;
    free_list->size = HEAP_SIZE - sizeof(struct block_header);
    free_list->is_free = 1;
    free_list->next = NULL;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    // Align request size to 8 bytes
    size = (size + 7) & ~7;

    struct block_header* curr = free_list;
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            // Check if we can split this block
            if (curr->size >= size + sizeof(struct block_header) + 16) {
                struct block_header* next_block = (struct block_header*)((uint8_t*)curr + sizeof(struct block_header) + size);
                next_block->size = curr->size - size - sizeof(struct block_header);
                next_block->is_free = 1;
                next_block->next = curr->next;

                curr->size = size;
                curr->next = next_block;
            }
            curr->is_free = 0;
            return (void*)((uint8_t*)curr + sizeof(struct block_header));
        }
        curr = curr->next;
    }

    return NULL; // Out of heap memory
}

void kfree(void* ptr) {
    if (!ptr) return;

    struct block_header* block = (struct block_header*)((uint8_t*)ptr - sizeof(struct block_header));
    block->is_free = 1;

    // Merge adjacent free blocks
    struct block_header* curr = free_list;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(struct block_header) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}
