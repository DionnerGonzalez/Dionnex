/* ============================================================================
 * Dionnex Monolithic Kernel - Physical Memory Manager (PMM) Header
 * ============================================================================
 */
#ifndef _DIONNEX_PMM_H_
#define _DIONNEX_PMM_H_

#include <types.h>

#define PAGE_SIZE 4096

void pmm_init(multiboot_info_t *mbi);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t addr);
uint32_t pmm_get_free_count(void);
uint32_t pmm_get_total_count(void);

#endif /* _DIONNEX_PMM_H_ */
