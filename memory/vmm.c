/* ============================================================================
 * Dionnex Kernel - Virtual Memory Manager & Paging (memory/vmm.c)
 * ============================================================================
 */
#include <kernel.h>

#define PDE_PRESENT  0x01
#define PDE_RW       0x02
#define PDE_USER     0x04

#define PTE_PRESENT  0x01
#define PTE_RW       0x02
#define PTE_USER     0x04

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void vmm_init(void) {
    // Identity map the first 4 MB (0x00000000 - 0x003FFFFF)
    for (uint32_t i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 0x1000) | PTE_PRESENT | PTE_RW;
    }

    // Set Page Directory entry 0
    page_directory[0] = ((uint32_t)first_page_table) | PDE_PRESENT | PDE_RW;

    // Fill remaining page directory entries as not present
    for (uint32_t i = 1; i < 1024; i++) {
        page_directory[i] = 0 | PDE_RW;
    }

    // Load CR3 with Page Directory physical address
    __asm__ __volatile__("mov %0, %%cr3" : : "r"(page_directory));

    // Enable Paging bit (PG = bit 31) in CR0
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(cr0));

    vga_puts("[VMM] Paginacion virtual activada (CR0 bit PG=1, Identity map 0-4MB).\n");
}

int vmm_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = (vaddr >> 12) & 0x03FF;

    if (!(page_directory[pd_index] & PDE_PRESENT)) {
        // Allocate page table if not present
        page_directory[pd_index] = (paddr & 0xFFFFF000) | flags | PDE_PRESENT;
    }

    uint32_t* pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    pt[pt_index] = (paddr & 0xFFFFF000) | flags | PTE_PRESENT;

    // Flush TLB
    __asm__ __volatile__("invlpg (%0)" : : "r"(vaddr) : "memory");
    return 0;
}

void vmm_unmap_page(uint32_t vaddr) {
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = (vaddr >> 12) & 0x03FF;

    if (page_directory[pd_index] & PDE_PRESENT) {
        uint32_t* pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
        pt[pt_index] = 0;
        __asm__ __volatile__("invlpg (%0)" : : "r"(vaddr) : "memory");
    }
}
