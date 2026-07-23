/* ============================================================================
 * Dionnex Monolithic Kernel - Virtual Memory Manager (X86 Paging Engine)
 * ============================================================================
 */
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <kernel/printk.h>

#define PAGE_DIR_INDEX(virt) (((virt) >> 22) & 0x3FF)
#define PAGE_TBL_INDEX(virt) (((virt) >> 12) & 0x3FF)

static uint32_t *page_directory = 0;

void vmm_init(void) {
    page_directory = (uint32_t*)pmm_alloc_page();
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }

    /* Identity map first 4MB */
    uint32_t *first_pt = (uint32_t*)pmm_alloc_page();
    for (int i = 0; i < 1024; i++) {
        first_pt[i] = (i * PAGE_SIZE) | VMM_PRESENT | VMM_WRITABLE;
    }

    page_directory[0] = ((uint32_t)first_pt) | VMM_PRESENT | VMM_WRITABLE;

    /* Enable x86 Paging Hardware */
    asm volatile("mov %0, %%cr3" : : "r"(page_directory));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // PG bit
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    printk("VMM: Paging enabled, identity mapped 4MB\n");
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_idx = PAGE_DIR_INDEX(virt);
    uint32_t pt_idx = PAGE_TBL_INDEX(virt);

    if (!(page_directory[pd_idx] & VMM_PRESENT)) {
        uint32_t *new_pt = (uint32_t*)pmm_alloc_page();
        for (int i = 0; i < 1024; i++) {
            new_pt[i] = 0;
        }
        page_directory[pd_idx] = ((uint32_t)new_pt) | VMM_PRESENT | VMM_WRITABLE | (flags & VMM_USER);
    }

    uint32_t *pt = (uint32_t*)(page_directory[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = (phys & 0xFFFFF000) | flags | VMM_PRESENT;

    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_unmap_page(uint32_t virt) {
    uint32_t pd_idx = PAGE_DIR_INDEX(virt);
    if (page_directory[pd_idx] & VMM_PRESENT) {
        uint32_t *pt = (uint32_t*)(page_directory[pd_idx] & 0xFFFFF000);
        pt[PAGE_TBL_INDEX(virt)] = 0;
        asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
    }
}

uint32_t vmm_get_phys(uint32_t virt) {
    uint32_t pd_idx = PAGE_DIR_INDEX(virt);
    if (!(page_directory[pd_idx] & VMM_PRESENT)) return 0;

    uint32_t *pt = (uint32_t*)(page_directory[pd_idx] & 0xFFFFF000);
    uint32_t entry = pt[PAGE_TBL_INDEX(virt)];

    if (!(entry & VMM_PRESENT)) return 0;
    return (entry & 0xFFFFF000) | (virt & 0xFFF);
}
