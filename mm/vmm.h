/* ============================================================================
 * Dionnex Monolithic Kernel - Virtual Memory Manager (VMM) Header
 * ============================================================================
 */
#ifndef _DIONNEX_VMM_H_
#define _DIONNEX_VMM_H_

#include <types.h>

#define VMM_PRESENT  0x001U
#define VMM_WRITABLE 0x002U
#define VMM_USER     0x004U
#define VMM_PWT      0x008U
#define VMM_PCD      0x010U
#define VMM_ACCESSED 0x020U
#define VMM_DIRTY    0x040U
#define VMM_NOCACHE  (VMM_PWT | VMM_PCD)

#define VMM_IOREMAP_BASE  0xD0000000U
#define VMM_IOREMAP_LIMIT 0xE0000000U
#define VMM_TEMP_KMAP     0xFF000000U

typedef struct page_directory {
    uint32_t entries[1024];
} __attribute__((aligned(4096))) page_directory_t;

typedef struct page_table {
    uint32_t entries[1024];
} __attribute__((aligned(4096))) page_table_t;

void vmm_init(void);

/* Active/kernel address space operations */
void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void vmm_unmap_page(uint32_t virt);
uint32_t vmm_get_phys(uint32_t virt);

/* Per-directory address space operations */
page_directory_t *vmm_get_kernel_directory(void);
page_directory_t *vmm_get_current_directory(void);
void vmm_switch_directory(page_directory_t *pd);

page_directory_t *vmm_create_address_space(void);
void vmm_destroy_address_space(page_directory_t *pd);
page_directory_t *vmm_clone_address_space(page_directory_t *src_pd);

int vmm_map_page_in(page_directory_t *pd, uint32_t virt, uint32_t phys, uint32_t flags);
void vmm_unmap_page_in(page_directory_t *pd, uint32_t virt);
uint32_t vmm_get_phys_in(page_directory_t *pd, uint32_t virt);
uint32_t vmm_get_flags_in(page_directory_t *pd, uint32_t virt);

/* Physical frame read/write helper independent of active CR3 */
int vmm_write_phys_page(uint32_t phys_page, uint32_t page_offset, const void *src, uint32_t len);
int vmm_zero_phys_page(uint32_t phys_page);

/* MMIO & hardware table mapping */
void *vmm_ioremap(uint32_t phys_addr, uint32_t size);
int vmm_map_mmio(uint32_t phys_addr, uint32_t size);

/* User pointer security validation */
int vmm_validate_user_range(page_directory_t *pd, uint32_t addr, uint32_t len, int write_required);
int vmm_validate_user_string(page_directory_t *pd, const char *str, uint32_t max_len);

#endif /* _DIONNEX_VMM_H_ */
