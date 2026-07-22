/* ============================================================================
 * Dionnex Kernel - Heap Dynamic Allocator (memory/kmalloc.c)
 * ============================================================================
 */
#include <kernel.h>

#define HEAP_START 0x01000000 // 16 MB boundary
#define HEAP_SIZE  0x00400000 // 4 MB kernel heap size

struct block_header {
    size_t size;
    uint8_t is_free;
    struct block_header* next;
};

static struct block_header* heap_head = NULL;

void kmalloc_init(void) {
    heap_head = (struct block_header*)HEAP_START;
    heap_head->size = HEAP_SIZE - sizeof(struct block_header);
    heap_head->is_free = 1;
    heap_head->next = NULL;

    vga_puts("[KMALLOC] Heap dinámico de kernel inicializado en 0x01000000 (4MB).\n");
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    // Align size to 8 bytes
    size = (size + 7) & ~7;

    struct block_header* curr = heap_head;
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            // Can we split block?
            if (curr->size >= size + sizeof(struct block_header) + 16) {
                struct block_header* new_block = (struct block_header*)((uint8_t*)curr + sizeof(struct block_header) + size);
                new_block->size = curr->size - size - sizeof(struct block_header);
                new_block->is_free = 1;
                new_block->next = curr->next;

                curr->size = size;
                curr->next = new_block;
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
    struct block_header* curr = heap_head;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(struct block_header) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}
