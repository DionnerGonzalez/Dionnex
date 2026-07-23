/* ============================================================================
 * Dionnex Monolithic Kernel - Virtual Memory Manager (VMM) Header
 * ============================================================================
 */
#ifndef _DIONNEX_VMM_H_
#define _DIONNEX_VMM_H_

#include <types.h>

#define VMM_PRESENT  0x1
#define VMM_WRITABLE 0x2
#define VMM_USER     0x4

void vmm_init(void);
void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void vmm_unmap_page(uint32_t virt);
uint32_t vmm_get_phys(uint32_t virt);

#endif /* _DIONNEX_VMM_H_ */
