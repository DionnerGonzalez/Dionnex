/* ============================================================================
 * Dionnex Kernel - Physical Memory Manager Bitmap (memory/pmm.c)
 * ============================================================================
 */
#include <kernel.h>

#define PAGE_SIZE 4096
#define MAX_PHYS_PAGES 32768 // 128 MB RAM / 4 KB = 32768 pages

static uint32_t pmm_bitmap[MAX_PHYS_PAGES / 32];
static uint32_t total_pages = MAX_PHYS_PAGES;
static uint32_t used_pages = 0;

static inline void bitmap_set(uint32_t bit) {
    pmm_bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void bitmap_clear(uint32_t bit) {
    pmm_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline int bitmap_test(uint32_t bit) {
    return pmm_bitmap[bit / 32] & (1 << (bit % 32));
}

void pmm_init(uint32_t mem_size_mb) {
    total_pages = (mem_size_mb * 1024 * 1024) / PAGE_SIZE;
    if (total_pages > MAX_PHYS_PAGES) total_pages = MAX_PHYS_PAGES;

    for (uint32_t i = 0; i < (total_pages / 32); i++) {
        pmm_bitmap[i] = 0; // Clear bitmap (all pages free initially)
    }

    // Reserve first 1 MB (256 pages) for BIOS, VGA, Kernel boot code
    for (uint32_t i = 0; i < 256; i++) {
        bitmap_set(i);
        used_pages++;
    }

    vga_puts("[PMM] Physical Memory Manager (Bitmap) listo: 128MB RAM, 32768 paginas (4KB).\n");
}

void* pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL; // Out of physical memory
}

void pmm_free_page(void* paddr) {
    uint32_t page_idx = (uint32_t)paddr / PAGE_SIZE;
    if (page_idx < total_pages && bitmap_test(page_idx)) {
        bitmap_clear(page_idx);
        used_pages--;
    }
}

uint32_t pmm_get_used_pages(void) {
    return used_pages;
}

uint32_t pmm_get_total_pages(void) {
    return total_pages;
}
