/* ============================================================================
 * Dionnex Kernel - Physical Memory Manager (memory/pmm.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 32768 // 128 MB

static uint32_t page_bitmap[MAX_PAGES / 32];
static uint32_t total_pages = 0;
static uint32_t used_pages = 0;

static inline void bitmap_set(uint32_t bit) {
    page_bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void bitmap_clear(uint32_t bit) {
    page_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline int bitmap_test(uint32_t bit) {
    return (page_bitmap[bit / 32] & (1 << (bit % 32))) != 0;
}

void pmm_init(uint32_t mem_size_mb) {
    total_pages = (mem_size_mb * 1024 * 1024) / PAGE_SIZE;
    if (total_pages > MAX_PAGES) total_pages = MAX_PAGES;

    memset(page_bitmap, 0, sizeof(page_bitmap));
    used_pages = 0;

    // Mark first 1MB (256 pages) as reserved for BIOS/VGA/Kernel low memory
    for (uint32_t i = 0; i < 256; i++) {
        bitmap_set(i);
        used_pages++;
    }
}

void* pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL; // Out of memory
}

void pmm_free_page(void* paddr) {
    uint32_t page = (uint32_t)paddr / PAGE_SIZE;
    if (page < total_pages && bitmap_test(page)) {
        bitmap_clear(page);
        if (used_pages > 0) used_pages--;
    }
}

uint32_t pmm_get_used_pages(void) {
    return used_pages;
}

uint32_t pmm_get_total_pages(void) {
    return total_pages;
}
