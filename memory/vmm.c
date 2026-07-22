/* ============================================================================
 * Dionnex Kernel - Virtual Memory Manager & Page Directory (memory/vmm.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

#define PAGE_SIZE 4096
#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024

typedef uint32_t pde_t;
typedef uint32_t pte_t;

static pde_t kernel_page_directory[TABLES_PER_DIR] __attribute__((aligned(4096)));
static pte_t first_page_table[PAGES_PER_TABLE] __attribute__((aligned(4096)));

void vmm_init(void) {
    memset(kernel_page_directory, 0, sizeof(kernel_page_directory));

    // Identity map the first 4MB of RAM using first_page_table
    for (uint32_t i = 0; i < PAGES_PER_TABLE; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | 0x3; // Supervisor, Read/Write, Present
    }

    // Put first page table into page directory entry 0
    kernel_page_directory[0] = ((uint32_t)first_page_table) | 0x3;

    // Load CR3 with page directory physical address
    uint32_t pd_addr = (uint32_t)kernel_page_directory;
    __asm__ __volatile__ ("mov %0, %%cr3" : : "r"(pd_addr));

    // Enable paging in CR0 (Bit 31 PG = 1)
    uint32_t cr0;
    __asm__ __volatile__ ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ __volatile__ ("mov %0, %%cr0" : : "r"(cr0));
}

int vmm_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    uint32_t pd_idx = vaddr >> 22;
    uint32_t pt_idx = (vaddr >> 12) & 0x3FF;

    if (!(kernel_page_directory[pd_idx] & 0x1)) {
        // Allocate a new page table
        pte_t* new_pt = (pte_t*)pmm_alloc_page();
        if (!new_pt) return -1;
        memset(new_pt, 0, PAGE_SIZE);
        kernel_page_directory[pd_idx] = ((uint32_t)new_pt) | 0x7; // User, RW, Present
    }

    pte_t* pt = (pte_t*)(kernel_page_directory[pd_idx] & ~0xFFF);
    pt[pt_idx] = (paddr & ~0xFFF) | (flags & 0xFFF) | 0x1;

    // Invalidate TLB for virtual address
    __asm__ __volatile__ ("invlpg (%0)" : : "r"(vaddr) : "memory");

    return 0;
}

void vmm_unmap_page(uint32_t vaddr) {
    uint32_t pd_idx = vaddr >> 22;
    uint32_t pt_idx = (vaddr >> 12) & 0x3FF;

    if (kernel_page_directory[pd_idx] & 0x1) {
        pte_t* pt = (pte_t*)(kernel_page_directory[pd_idx] & ~0xFFF);
        pt[pt_idx] = 0;
        __asm__ __volatile__ ("invlpg (%0)" : : "r"(vaddr) : "memory");
    }
}
