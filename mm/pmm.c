/* ============================================================================
 * Dionnex Monolithic Kernel - Physical Memory Manager (Bitmap Allocator)
 * ============================================================================
 */
#include <mm/pmm.h>
#include <kernel/printk.h>

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

static uint32_t *bitmap = 0;
static uint32_t bitmap_size = 0; // In 32-bit uint32_t words
static uint32_t total_pages = 0;
static uint32_t free_pages = 0;

#define PAGE_INDEX(addr) ((addr) / PAGE_SIZE)
#define BIT_INDEX(page)  ((page) / 32)
#define BIT_OFFSET(page) ((page) % 32)

static inline void bitmap_set(uint32_t page) {
    bitmap[BIT_INDEX(page)] |= (1U << BIT_OFFSET(page));
}

static inline void bitmap_clear(uint32_t page) {
    bitmap[BIT_INDEX(page)] &= ~(1U << BIT_OFFSET(page));
}

static inline int bitmap_test(uint32_t page) {
    return (bitmap[BIT_INDEX(page)] >> BIT_OFFSET(page)) & 1U;
}

void pmm_init(multiboot_info_t *mbi) {
    uint32_t max_addr = 0x1000000; // Default 16MB fallback if mmap absent

    if (mbi && (mbi->flags & (1 << 6)) && mbi->mmap_length > 0) {
        multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t*)mbi->mmap_addr;
        uint32_t end_mmap = mbi->mmap_addr + mbi->mmap_length;

        while ((uint32_t)entry < end_mmap) {
            if (entry->type == 1) { // Available RAM
                uint32_t entry_end = entry->addr_low + entry->len_low;
                if (entry_end > max_addr) {
                    max_addr = entry_end;
                }
            }
            entry = (multiboot_mmap_entry_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
        }
    } else if (mbi && (mbi->flags & (1 << 0))) {
        max_addr = (mbi->mem_upper + 1024) * 1024;
    }

    total_pages = max_addr / PAGE_SIZE;
    bitmap_size = (total_pages + 31) / 32;

    bitmap = (uint32_t*)ALIGN_UP((uint32_t)&_kernel_end, PAGE_SIZE);

    // Initially mark ALL pages as reserved/used
    for (uint32_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }
    free_pages = 0;

    // Unmark available memory regions according to Multiboot memory map
    if (mbi && (mbi->flags & (1 << 6)) && mbi->mmap_length > 0) {
        multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t*)mbi->mmap_addr;
        uint32_t end_mmap = mbi->mmap_addr + mbi->mmap_length;

        while ((uint32_t)entry < end_mmap) {
            if (entry->type == 1) {
                uint32_t start_page = PAGE_INDEX(entry->addr_low);
                uint32_t page_count = entry->len_low / PAGE_SIZE;

                for (uint32_t p = 0; p < page_count; p++) {
                    uint32_t page = start_page + p;
                    if (page < total_pages && bitmap_test(page)) {
                        bitmap_clear(page);
                        free_pages++;
                    }
                }
            }
            entry = (multiboot_mmap_entry_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
        }
    } else {
        // Fallback: 1MB to max_addr marked free
        uint32_t start_page = PAGE_INDEX(0x100000);
        for (uint32_t page = start_page; page < total_pages; page++) {
            bitmap_clear(page);
            free_pages++;
        }
    }

    // Reserve Page 0 (Null guard page)
    if (!bitmap_test(0)) {
        bitmap_set(0);
        if (free_pages > 0) free_pages--;
    }

    // Reserve Kernel space
    uint32_t kstart_page = PAGE_INDEX((uint32_t)&_kernel_start);
    uint32_t kend_page   = PAGE_INDEX((uint32_t)&_kernel_end);
    for (uint32_t p = kstart_page; p <= kend_page; p++) {
        if (!bitmap_test(p)) {
            bitmap_set(p);
            if (free_pages > 0) free_pages--;
        }
    }

    // Reserve Bitmap structure space
    uint32_t bstart_page = PAGE_INDEX((uint32_t)bitmap);
    uint32_t bend_page   = PAGE_INDEX((uint32_t)bitmap + (bitmap_size * sizeof(uint32_t)));
    for (uint32_t p = bstart_page; p <= bend_page; p++) {
        if (!bitmap_test(p)) {
            bitmap_set(p);
            if (free_pages > 0) free_pages--;
        }
    }

    printk("PMM: %u KB total, %u KB free, %u KB used\n",
           total_pages * 4, free_pages * 4, (total_pages - free_pages) * 4);
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < bitmap_size; i++) {
        if (bitmap[i] != 0xFFFFFFFF) { // Contains at least one free bit
            for (int j = 0; j < 32; j++) {
                if (!(bitmap[i] & (1U << j))) {
                    uint32_t page = i * 32 + j;
                    if (page >= total_pages) return 0;

                    bitmap_set(page);
                    if (free_pages > 0) free_pages--;
                    return page * PAGE_SIZE;
                }
            }
        }
    }
    return 0; // Out of memory
}

void pmm_free_page(uint32_t addr) {
    uint32_t page = PAGE_INDEX(addr);
    if (page < total_pages && bitmap_test(page)) {
        bitmap_clear(page);
        free_pages++;
    }
}

uint32_t pmm_get_free_count(void) {
    return free_pages;
}

uint32_t pmm_get_total_count(void) {
    return total_pages;
}
