/* ============================================================================
 * Dionnex Monolithic Kernel - Physical Memory Manager (Bitmap Allocator)
 * ============================================================================
 */
#include <mm/pmm.h>
#include <kernel/multiboot.h>
#include <kernel/printk.h>
#include <string.h>

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

static uint32_t *bitmap = NULL;
static uint32_t bitmap_size = 0;   /* Number of 32-bit words */
static uint32_t total_pages = 0;
static uint32_t free_pages = 0;
static uint32_t managed_top = 0;
static uint32_t bitmap_end_addr = 0;

/* Cap 32-bit managed physical RAM at 1 GB to keep bitmap compact in low RAM */
#define PMM_MAX_PHYS_ADDR 0x40000000ULL

#define PAGE_INDEX(addr) ((addr) / PAGE_SIZE)
#define BIT_INDEX(page)  ((page) / 32U)
#define BIT_OFFSET(page) ((page) % 32U)

static inline void bitmap_set(uint32_t page) {
    if (page < total_pages) {
        bitmap[BIT_INDEX(page)] |= (1U << BIT_OFFSET(page));
    }
}

static inline void bitmap_clear(uint32_t page) {
    if (page < total_pages) {
        bitmap[BIT_INDEX(page)] &= ~(1U << BIT_OFFSET(page));
    }
}

static inline int bitmap_test(uint32_t page) {
    if (page >= total_pages) return 1;
    return (int)((bitmap[BIT_INDEX(page)] >> BIT_OFFSET(page)) & 1U);
}

static void pmm_mark_used(uint32_t page) {
    if (page < total_pages && !bitmap_test(page)) {
        bitmap_set(page);
        if (free_pages > 0) free_pages--;
    }
}

static void pmm_mark_free(uint32_t page) {
    if (page < total_pages && bitmap_test(page)) {
        bitmap_clear(page);
        free_pages++;
    }
}

void pmm_reserve_region(uint32_t start, uint32_t size) {
    if (size == 0) return;
    uint64_t end64 = (uint64_t)start + (uint64_t)size;
    if (end64 > managed_top) end64 = managed_top;
    if ((uint64_t)start >= end64) return;

    uint32_t first_page = start / PAGE_SIZE;
    uint32_t last_page = (uint32_t)((end64 + PAGE_SIZE - 1) / PAGE_SIZE);
    if (last_page > total_pages) last_page = total_pages;

    for (uint32_t p = first_page; p < last_page; p++) {
        pmm_mark_used(p);
    }
}

void pmm_free_region(uint32_t start, uint32_t size) {
    if (size == 0) return;
    uint64_t s64 = ALIGN_UP((uint64_t)start, PAGE_SIZE);
    uint64_t e64 = ALIGN_DOWN((uint64_t)start + (uint64_t)size, PAGE_SIZE);
    if (e64 > managed_top) e64 = ALIGN_DOWN((uint64_t)managed_top, PAGE_SIZE);
    if (s64 >= e64) return;

    uint32_t first_page = (uint32_t)(s64 / PAGE_SIZE);
    uint32_t last_page = (uint32_t)(e64 / PAGE_SIZE);
    if (last_page > total_pages) last_page = total_pages;

    for (uint32_t p = first_page; p < last_page; p++) {
        pmm_mark_free(p);
    }
}

void pmm_init(multiboot_info_t *mbi) {
    uint64_t max_addr = 0x1000000ULL; /* Default 16MB fallback */

    if (mbi && (mbi->flags & MULTIBOOT_INFO_MEM_MAP) && mbi->mmap_length > 0) {
        uint32_t curr_addr = mbi->mmap_addr;
        uint32_t end_mmap = mbi->mmap_addr + mbi->mmap_length;

        while (curr_addr < end_mmap) {
            multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t*)curr_addr;
            uint64_t base = ((uint64_t)entry->addr_high << 32) | entry->addr_low;
            uint64_t len  = ((uint64_t)entry->len_high << 32)  | entry->len_low;

            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE && base < PMM_MAX_PHYS_ADDR) {
                uint64_t region_end = base + len;
                if (region_end > PMM_MAX_PHYS_ADDR) {
                    region_end = PMM_MAX_PHYS_ADDR;
                }
                if (region_end > max_addr) {
                    max_addr = region_end;
                }
            }
            if (entry->size == 0) break;
            curr_addr += entry->size + sizeof(entry->size);
        }
    } else if (mbi && (mbi->flags & MULTIBOOT_INFO_MEMORY)) {
        uint64_t upper = ((uint64_t)mbi->mem_upper + 1024ULL) * 1024ULL;
        if (upper > PMM_MAX_PHYS_ADDR) upper = PMM_MAX_PHYS_ADDR;
        if (upper > max_addr) max_addr = upper;
    }

    managed_top = (uint32_t)ALIGN_DOWN(max_addr, PAGE_SIZE);
    total_pages = managed_top / PAGE_SIZE;
    bitmap_size = (total_pages + 31U) / 32U;

    /* Determine safe placement for PMM bitmap after kernel and multiboot mods */
    uint32_t safe_start = ALIGN_UP((uint32_t)&_kernel_end, PAGE_SIZE);
    if (mbi) {
        uint32_t mbi_end = ALIGN_UP((uint32_t)mbi + sizeof(multiboot_info_t), PAGE_SIZE);
        if (mbi_end > safe_start && mbi_end < 0x02000000U) safe_start = mbi_end;

        if ((mbi->flags & MULTIBOOT_INFO_MEM_MAP) && mbi->mmap_addr) {
            uint32_t mmap_end = ALIGN_UP(mbi->mmap_addr + mbi->mmap_length, PAGE_SIZE);
            if (mmap_end > safe_start && mmap_end < 0x02000000U) safe_start = mmap_end;
        }
        if ((mbi->flags & MULTIBOOT_INFO_MODS) && mbi->mods_addr && mbi->mods_count > 0) {
            multiboot_module_t *mods = (multiboot_module_t*)mbi->mods_addr;
            uint32_t mods_end = ALIGN_UP(mbi->mods_addr + mbi->mods_count * sizeof(multiboot_module_t), PAGE_SIZE);
            if (mods_end > safe_start && mods_end < 0x02000000U) safe_start = mods_end;
            for (uint32_t m = 0; m < mbi->mods_count; m++) {
                uint32_t mend = ALIGN_UP(mods[m].mod_end, PAGE_SIZE);
                if (mend > safe_start && mend < 0x02000000U) safe_start = mend;
            }
        }
    }

    bitmap = (uint32_t*)safe_start;
    bitmap_end_addr = ALIGN_UP(safe_start + bitmap_size * sizeof(uint32_t), PAGE_SIZE);

    /* Initially mark ALL pages used */
    for (uint32_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0xFFFFFFFFU;
    }
    free_pages = 0;

    /* Unmark available RAM regions from Multiboot memory map */
    if (mbi && (mbi->flags & MULTIBOOT_INFO_MEM_MAP) && mbi->mmap_length > 0) {
        uint32_t curr_addr = mbi->mmap_addr;
        uint32_t end_mmap = mbi->mmap_addr + mbi->mmap_length;

        while (curr_addr < end_mmap) {
            multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t*)curr_addr;
            uint64_t base = ((uint64_t)entry->addr_high << 32) | entry->addr_low;
            uint64_t len  = ((uint64_t)entry->len_high << 32)  | entry->len_low;

            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE && base < managed_top) {
                uint64_t region_end = base + len;
                if (region_end > managed_top) region_end = managed_top;
                if (region_end > base) {
                    pmm_free_region((uint32_t)base, (uint32_t)(region_end - base));
                }
            }
            if (entry->size == 0) break;
            curr_addr += entry->size + sizeof(entry->size);
        }
    } else {
        if (managed_top > 0x100000U) {
            pmm_free_region(0x100000U, managed_top - 0x100000U);
        }
    }

    /* Reserve low 1MB (IVT, BDA, EBDA, VGA text/graphics, BIOS ROM) */
    pmm_reserve_region(0x00000000U, 0x00100000U);

    /* Reserve kernel image */
    pmm_reserve_region((uint32_t)&_kernel_start,
                       (uint32_t)&_kernel_end - (uint32_t)&_kernel_start);

    /* Reserve PMM bitmap */
    pmm_reserve_region((uint32_t)bitmap, bitmap_end_addr - (uint32_t)bitmap);

    /* Reserve Multiboot structures and modules */
    if (mbi) {
        pmm_reserve_region((uint32_t)mbi, sizeof(multiboot_info_t));
        if ((mbi->flags & MULTIBOOT_INFO_MEM_MAP) && mbi->mmap_addr) {
            pmm_reserve_region(mbi->mmap_addr, mbi->mmap_length);
        }
        if ((mbi->flags & MULTIBOOT_INFO_MODS) && mbi->mods_addr && mbi->mods_count > 0) {
            pmm_reserve_region(mbi->mods_addr, mbi->mods_count * sizeof(multiboot_module_t));
            multiboot_module_t *mods = (multiboot_module_t*)mbi->mods_addr;
            for (uint32_t m = 0; m < mbi->mods_count; m++) {
                if (mods[m].mod_end > mods[m].mod_start) {
                    pmm_reserve_region(mods[m].mod_start, mods[m].mod_end - mods[m].mod_start);
                }
            }
        }
        if ((mbi->flags & MULTIBOOT_INFO_VBE_INFO) && mbi->vbe_mode_info) {
            pmm_reserve_region(mbi->vbe_mode_info, 256);
        }
    }

    printk("PMM: %u KB total (%u MB), %u KB free, bitmap at 0x%x\n",
           total_pages * 4, managed_top / (1024 * 1024), free_pages * 4, (uint32_t)bitmap);
}

static uint32_t pmm_alloc_pages_range(uint32_t start_page, uint32_t end_page, uint32_t count) {
    if (count == 0 || start_page >= end_page) return 0;
    if (end_page > total_pages) end_page = total_pages;

    uint32_t run = 0;
    uint32_t run_start = 0;

    for (uint32_t page = start_page; page < end_page; page++) {
        /* Fast word skip when looking for a new run */
        if (run == 0 && (page % 32U) == 0 && page + 32U <= end_page) {
            if (bitmap[BIT_INDEX(page)] == 0xFFFFFFFFU) {
                page += 31U;
                continue;
            }
        }

        if (!bitmap_test(page)) {
            if (run == 0) run_start = page;
            run++;
            if (run == count) {
                for (uint32_t k = 0; k < count; k++) {
                    pmm_mark_used(run_start + k);
                }
                return run_start * PAGE_SIZE;
            }
        } else {
            run = 0;
        }
    }
    return 0;
}

uint32_t pmm_alloc_low_pages(uint32_t count) {
    uint32_t low_limit_page = PAGE_INDEX(PMM_LOW_MEM_LIMIT);
    if (low_limit_page > total_pages) low_limit_page = total_pages;
    return pmm_alloc_pages_range(1, low_limit_page, count);
}

uint32_t pmm_alloc_low_page(void) {
    return pmm_alloc_low_pages(1);
}

uint32_t pmm_alloc_pages(uint32_t count) {
    if (count == 0) return 0;
    uint32_t low_limit_page = PAGE_INDEX(PMM_LOW_MEM_LIMIT);
    if (total_pages > low_limit_page) {
        uint32_t addr = pmm_alloc_pages_range(low_limit_page, total_pages, count);
        if (addr != 0) return addr;
    }
    return pmm_alloc_pages_range(1, total_pages, count);
}

uint32_t pmm_alloc_page(void) {
    return pmm_alloc_pages(1);
}

void pmm_free_pages(uint32_t addr, uint32_t count) {
    if (addr == 0 || count == 0) return;
    uint32_t start_page = PAGE_INDEX(addr);
    for (uint32_t i = 0; i < count; i++) {
        uint32_t page = start_page + i;
        if (page > 0 && page < total_pages) {
            pmm_mark_free(page);
        }
    }
}

void pmm_free_page(uint32_t addr) {
    pmm_free_pages(addr, 1);
}

uint32_t pmm_get_free_count(void) {
    return free_pages;
}

uint32_t pmm_get_total_count(void) {
    return total_pages;
}

uint32_t pmm_get_managed_top(void) {
    return managed_top;
}

uint32_t pmm_get_bitmap_end(void) {
    return bitmap_end_addr;
}
