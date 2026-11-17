/* ============================================================================
 * Dionnex Monolithic Kernel - Physical Memory Manager (PMM) Header
 * ============================================================================
 */
#ifndef _DIONNEX_PMM_H_
#define _DIONNEX_PMM_H_

#include <types.h>

#define PAGE_SIZE         4096
#define PMM_LOW_MEM_LIMIT 0x04000000U /* 64 MB identity-mapped kernel/DMA window */

void pmm_init(multiboot_info_t *mbi);

/* Single page allocation (prefers high RAM >= 64MB, falls back to low RAM) */
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t addr);

/* Low identity-mapped (< 64MB) contiguous page allocation for kernel/heap/DMA */
uint32_t pmm_alloc_low_page(void);
uint32_t pmm_alloc_low_pages(uint32_t count);

/* Contiguous multi-page physical allocation */
uint32_t pmm_alloc_pages(uint32_t count);
void pmm_free_pages(uint32_t addr, uint32_t count);

/* Region reservation & release */
void pmm_reserve_region(uint32_t start, uint32_t size);
void pmm_free_region(uint32_t start, uint32_t size);

/* Diagnostics */
uint32_t pmm_get_free_count(void);
uint32_t pmm_get_total_count(void);
uint32_t pmm_get_managed_top(void);
uint32_t pmm_get_bitmap_end(void);

#endif /* _DIONNEX_PMM_H_ */
