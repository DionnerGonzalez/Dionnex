/* ============================================================================
 * Dionnex Monolithic Kernel - Virtual Memory Manager (x86 Paging Engine)
 * ============================================================================
 */
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <arch.h>
#include <kernel/idt.h>
#include <kernel/process.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <string.h>
#include <io.h>

#define PAGE_DIR_INDEX(virt) (((virt) >> 22) & 0x3FFU)
#define PAGE_TBL_INDEX(virt) (((virt) >> 12) & 0x3FFU)
#define PAGE_ADDR_MASK       0xFFFFF000U

static page_directory_t *kernel_directory = NULL;
static page_directory_t *current_directory = NULL;
static uint32_t ioremap_cursor = VMM_IOREMAP_BASE;
static int vmm_ready = 0;

static inline void vmm_invlpg(uint32_t virt) {
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

static void vmm_page_fault_handler(registers_t *regs) {
    uint32_t fault_addr = read_cr2();
    int present  = (regs->err_code & 0x1) != 0;
    int write    = (regs->err_code & 0x2) != 0;
    int user     = (regs->err_code & 0x4) != 0;
    int reserved = (regs->err_code & 0x8) != 0;
    int ifetch   = (regs->err_code & 0x10) != 0;

    if (user && current_process) {
        printk("Page Fault in Ring 3 process '%s' (pid %u) at 0x%08x (EIP 0x%08x) [%s %s %s]\n",
               current_process->name, current_process->pid, fault_addr, regs->eip,
               present ? "protection" : "non-present",
               write ? "write" : "read",
               ifetch ? "exec" : "data");
        process_exit(139); /* SIGSEGV */
        return;
    }

    printk("\nKERNEL PAGE FAULT at 0x%08x (EIP: 0x%08x, err: 0x%x)\n",
           fault_addr, regs->eip, regs->err_code);
    printk("  Flags: present=%d write=%d user=%d rsvd=%d ifetch=%d CR3=0x%08x\n",
           present, write, user, reserved, ifetch, read_cr3());
    kernel_panic("Kernel Page Fault (#PF)");
}

void vmm_init(void) {
    kernel_directory = (page_directory_t*)pmm_alloc_low_page();
    if (!kernel_directory) {
        kernel_panic("VMM: Failed to allocate kernel page directory");
    }
    memset(kernel_directory, 0, PAGE_SIZE);

    /* Identity-map low kernel/DMA window: up to min(managed_top, 64MB), minimum 16MB */
    uint32_t map_top = pmm_get_managed_top();
    if (map_top > PMM_LOW_MEM_LIMIT) map_top = PMM_LOW_MEM_LIMIT;
    if (map_top < 0x01000000U) map_top = 0x01000000U;

    uint32_t num_pdes = (map_top + 0x003FFFFFU) >> 22;
    for (uint32_t pd_idx = 0; pd_idx < num_pdes; pd_idx++) {
        page_table_t *pt = (page_table_t*)pmm_alloc_low_page();
        if (!pt) {
            kernel_panic("VMM: Failed to allocate initial page table");
        }
        uint32_t phys_base = pd_idx * 0x00400000U;
        for (uint32_t pt_idx = 0; pt_idx < 1024; pt_idx++) {
            uint32_t phys = phys_base + (pt_idx * PAGE_SIZE);
            /* Page 0 remains unmapped as NULL pointer guard unless explicitly mapped */
            if (phys == 0) {
                pt->entries[pt_idx] = 0;
            } else {
                pt->entries[pt_idx] = phys | VMM_PRESENT | VMM_WRITABLE;
            }
        }
        kernel_directory->entries[pd_idx] = ((uint32_t)pt) | VMM_PRESENT | VMM_WRITABLE;
    }

    /* Pre-allocate high kernel PDEs (0xC0000000 .. 0xFFC00000) so user directories share them */
    for (uint32_t pd_idx = 768; pd_idx < 1024; pd_idx++) {
        page_table_t *pt = (page_table_t*)pmm_alloc_low_page();
        if (!pt) {
            kernel_panic("VMM: Failed to allocate high kernel page table");
        }
        memset(pt, 0, PAGE_SIZE);
        kernel_directory->entries[pd_idx] = ((uint32_t)pt) | VMM_PRESENT | VMM_WRITABLE;
    }

    current_directory = kernel_directory;
    ioremap_cursor = VMM_IOREMAP_BASE;

    /* Register #PF handler before enabling paging */
    isr_register_handler(14, vmm_page_fault_handler);

    /* Enable x86 paging */
    write_cr3((uint32_t)kernel_directory);
    uint32_t cr0 = read_cr0();
    cr0 |= 0x80000000U; /* PG */
    cr0 |= 0x00010000U; /* WP: enforce read-only kernel pages */
    write_cr0(cr0);

    vmm_ready = 1;
    printk("VMM: Paging enabled (%u MB low identity-map, kernel/user isolation active)\n",
           (num_pdes * 4));
}

page_directory_t *vmm_get_kernel_directory(void) {
    return kernel_directory;
}

page_directory_t *vmm_get_current_directory(void) {
    return current_directory;
}

void vmm_switch_directory(page_directory_t *pd) {
    if (!pd) pd = kernel_directory;
    if (!pd || pd == current_directory) return;
    current_directory = pd;
    write_cr3((uint32_t)pd);
}

int vmm_map_page_in(page_directory_t *pd, uint32_t virt, uint32_t phys, uint32_t flags) {
    if (!pd) pd = kernel_directory;
    if (!pd) return -1;

    uint32_t pd_idx = PAGE_DIR_INDEX(virt);
    uint32_t pt_idx = PAGE_TBL_INDEX(virt);

    if (!(pd->entries[pd_idx] & VMM_PRESENT)) {
        page_table_t *new_pt = (page_table_t*)pmm_alloc_low_page();
        if (!new_pt) return -1;
        memset(new_pt, 0, PAGE_SIZE);
        uint32_t pde_flags = VMM_PRESENT | VMM_WRITABLE | (flags & VMM_USER);
        pd->entries[pd_idx] = ((uint32_t)new_pt) | pde_flags;
    } else if (flags & VMM_USER) {
        pd->entries[pd_idx] |= VMM_USER;
    }

    page_table_t *pt = (page_table_t*)(pd->entries[pd_idx] & PAGE_ADDR_MASK);
    pt->entries[pt_idx] = (phys & PAGE_ADDR_MASK) | (flags & 0xFFFU) | VMM_PRESENT;

    if (vmm_ready && pd == current_directory) {
        vmm_invlpg(virt & PAGE_ADDR_MASK);
    }
    return 0;
}

void vmm_unmap_page_in(page_directory_t *pd, uint32_t virt) {
    if (!pd) pd = kernel_directory;
    if (!pd) return;

    uint32_t pd_idx = PAGE_DIR_INDEX(virt);
    if (pd->entries[pd_idx] & VMM_PRESENT) {
        page_table_t *pt = (page_table_t*)(pd->entries[pd_idx] & PAGE_ADDR_MASK);
        pt->entries[PAGE_TBL_INDEX(virt)] = 0;
        if (vmm_ready && pd == current_directory) {
            vmm_invlpg(virt & PAGE_ADDR_MASK);
        }
    }
}

uint32_t vmm_get_phys_in(page_directory_t *pd, uint32_t virt) {
    if (!pd) pd = kernel_directory;
    if (!pd) return 0;

    uint32_t pd_idx = PAGE_DIR_INDEX(virt);
    if (!(pd->entries[pd_idx] & VMM_PRESENT)) return 0;

    page_table_t *pt = (page_table_t*)(pd->entries[pd_idx] & PAGE_ADDR_MASK);
    uint32_t entry = pt->entries[PAGE_TBL_INDEX(virt)];
    if (!(entry & VMM_PRESENT)) return 0;

    return (entry & PAGE_ADDR_MASK) | (virt & 0xFFFU);
}

uint32_t vmm_get_flags_in(page_directory_t *pd, uint32_t virt) {
    if (!pd) pd = kernel_directory;
    if (!pd) return 0;

    uint32_t pd_idx = PAGE_DIR_INDEX(virt);
    if (!(pd->entries[pd_idx] & VMM_PRESENT)) return 0;

    page_table_t *pt = (page_table_t*)(pd->entries[pd_idx] & PAGE_ADDR_MASK);
    uint32_t entry = pt->entries[PAGE_TBL_INDEX(virt)];
    if (!(entry & VMM_PRESENT)) return 0;

    return entry & 0xFFFU;
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    vmm_map_page_in(kernel_directory, virt, phys, flags);
}

void vmm_unmap_page(uint32_t virt) {
    vmm_unmap_page_in(kernel_directory, virt);
}

uint32_t vmm_get_phys(uint32_t virt) {
    return vmm_get_phys_in(current_directory ? current_directory : kernel_directory, virt);
}

page_directory_t *vmm_create_address_space(void) {
    if (!kernel_directory) return NULL;

    page_directory_t *pd = (page_directory_t*)pmm_alloc_low_page();
    if (!pd) return NULL;
    memset(pd, 0, PAGE_SIZE);

    /* Share low kernel identity-mapped PDEs (0..64MB) - supervisor only */
    uint32_t low_pdes = PMM_LOW_MEM_LIMIT >> 22;
    for (uint32_t i = 0; i < low_pdes; i++) {
        pd->entries[i] = kernel_directory->entries[i];
    }

    /* Share high kernel PDEs (0xC0000000..0xFFFFFFFF: ioremap, MMIO, temp kmap) */
    for (uint32_t i = 768; i < 1024; i++) {
        pd->entries[i] = kernel_directory->entries[i];
    }

    return pd;
}

void vmm_destroy_address_space(page_directory_t *pd) {
    if (!pd || pd == kernel_directory) return;

    if (current_directory == pd) {
        vmm_switch_directory(kernel_directory);
    }

    uint32_t user_pd_start = PAGE_DIR_INDEX(DIONNEX_USER_VADDR_MIN);
    uint32_t user_pd_end   = PAGE_DIR_INDEX(DIONNEX_USER_VADDR_MAX);

    for (uint32_t pd_idx = user_pd_start; pd_idx < user_pd_end; pd_idx++) {
        if (pd->entries[pd_idx] & VMM_PRESENT) {
            page_table_t *pt = (page_table_t*)(pd->entries[pd_idx] & PAGE_ADDR_MASK);
            for (uint32_t pt_idx = 0; pt_idx < 1024; pt_idx++) {
                uint32_t entry = pt->entries[pt_idx];
                if ((entry & VMM_PRESENT) && (entry & VMM_USER)) {
                    pmm_free_page(entry & PAGE_ADDR_MASK);
                }
            }
            pmm_free_page((uint32_t)pt);
            pd->entries[pd_idx] = 0;
        }
    }

    pmm_free_page((uint32_t)pd);
}

int vmm_write_phys_page(uint32_t phys_page, uint32_t page_offset, const void *src, uint32_t len) {
    if (!src || page_offset + len > PAGE_SIZE) return -1;
    if (!vmm_ready) return -1;

    uint32_t pd_idx = PAGE_DIR_INDEX(VMM_TEMP_KMAP);
    uint32_t pt_idx = PAGE_TBL_INDEX(VMM_TEMP_KMAP);
    page_table_t *pt = (page_table_t*)(kernel_directory->entries[pd_idx] & PAGE_ADDR_MASK);

    pt->entries[pt_idx] = (phys_page & PAGE_ADDR_MASK) | VMM_PRESENT | VMM_WRITABLE;
    vmm_invlpg(VMM_TEMP_KMAP);

    memcpy((void*)(VMM_TEMP_KMAP + page_offset), src, len);

    pt->entries[pt_idx] = 0;
    vmm_invlpg(VMM_TEMP_KMAP);
    return 0;
}

int vmm_zero_phys_page(uint32_t phys_page) {
    if (!vmm_ready) return -1;

    uint32_t pd_idx = PAGE_DIR_INDEX(VMM_TEMP_KMAP);
    uint32_t pt_idx = PAGE_TBL_INDEX(VMM_TEMP_KMAP);
    page_table_t *pt = (page_table_t*)(kernel_directory->entries[pd_idx] & PAGE_ADDR_MASK);

    pt->entries[pt_idx] = (phys_page & PAGE_ADDR_MASK) | VMM_PRESENT | VMM_WRITABLE;
    vmm_invlpg(VMM_TEMP_KMAP);

    memset((void*)VMM_TEMP_KMAP, 0, PAGE_SIZE);

    pt->entries[pt_idx] = 0;
    vmm_invlpg(VMM_TEMP_KMAP);
    return 0;
}

static int vmm_copy_phys_page(uint32_t dst_phys, uint32_t src_phys) {
    uint32_t pd_idx = PAGE_DIR_INDEX(VMM_TEMP_KMAP);
    uint32_t pt_idx = PAGE_TBL_INDEX(VMM_TEMP_KMAP);
    page_table_t *pt = (page_table_t*)(kernel_directory->entries[pd_idx] & PAGE_ADDR_MASK);

    pt->entries[pt_idx]     = (src_phys & PAGE_ADDR_MASK) | VMM_PRESENT;
    pt->entries[pt_idx + 1] = (dst_phys & PAGE_ADDR_MASK) | VMM_PRESENT | VMM_WRITABLE;
    vmm_invlpg(VMM_TEMP_KMAP);
    vmm_invlpg(VMM_TEMP_KMAP + PAGE_SIZE);

    memcpy((void*)(VMM_TEMP_KMAP + PAGE_SIZE), (const void*)VMM_TEMP_KMAP, PAGE_SIZE);

    pt->entries[pt_idx] = 0;
    pt->entries[pt_idx + 1] = 0;
    vmm_invlpg(VMM_TEMP_KMAP);
    vmm_invlpg(VMM_TEMP_KMAP + PAGE_SIZE);
    return 0;
}

page_directory_t *vmm_clone_address_space(page_directory_t *src_pd) {
    if (!src_pd) return NULL;
    page_directory_t *dst_pd = vmm_create_address_space();
    if (!dst_pd) return NULL;

    uint32_t user_pd_start = PAGE_DIR_INDEX(DIONNEX_USER_VADDR_MIN);
    uint32_t user_pd_end   = PAGE_DIR_INDEX(DIONNEX_USER_VADDR_MAX);

    for (uint32_t pd_idx = user_pd_start; pd_idx < user_pd_end; pd_idx++) {
        if (!(src_pd->entries[pd_idx] & VMM_PRESENT)) continue;

        page_table_t *src_pt = (page_table_t*)(src_pd->entries[pd_idx] & PAGE_ADDR_MASK);
        for (uint32_t pt_idx = 0; pt_idx < 1024; pt_idx++) {
            uint32_t entry = src_pt->entries[pt_idx];
            if (!(entry & VMM_PRESENT) || !(entry & VMM_USER)) continue;

            uint32_t virt = (pd_idx << 22) | (pt_idx << 12);
            uint32_t src_phys = entry & PAGE_ADDR_MASK;
            uint32_t flags = entry & 0xFFFU;

            uint32_t dst_phys = pmm_alloc_page();
            if (!dst_phys) {
                vmm_destroy_address_space(dst_pd);
                return NULL;
            }
            vmm_copy_phys_page(dst_phys, src_phys);
            if (vmm_map_page_in(dst_pd, virt, dst_phys, flags) != 0) {
                pmm_free_page(dst_phys);
                vmm_destroy_address_space(dst_pd);
                return NULL;
            }
        }
    }

    return dst_pd;
}

void *vmm_ioremap(uint32_t phys_addr, uint32_t size) {
    if (size == 0) return NULL;

    uint32_t page_offset = phys_addr & 0xFFFU;
    uint32_t phys_base = phys_addr & PAGE_ADDR_MASK;
    uint32_t total_size = ALIGN_UP(page_offset + size, PAGE_SIZE);
    uint32_t pages = total_size / PAGE_SIZE;

    /* High physical MMIO (>= 0xC0000000) can also be identity-mapped in shared kernel PDEs */
    if (phys_base >= 0xC0000000U && (uint64_t)phys_base + total_size <= 0xFEFFFFFFULL) {
        for (uint32_t i = 0; i < pages; i++) {
            uint32_t addr = phys_base + i * PAGE_SIZE;
            vmm_map_page_in(kernel_directory, addr, addr, VMM_PRESENT | VMM_WRITABLE | VMM_NOCACHE);
        }
        return (void*)phys_addr;
    }

    if (ioremap_cursor + total_size > VMM_IOREMAP_LIMIT) {
        return NULL;
    }

    uint32_t virt_base = ioremap_cursor;
    ioremap_cursor += total_size;

    for (uint32_t i = 0; i < pages; i++) {
        if (vmm_map_page_in(kernel_directory, virt_base + i * PAGE_SIZE,
                            phys_base + i * PAGE_SIZE,
                            VMM_PRESENT | VMM_WRITABLE | VMM_NOCACHE) != 0) {
            return NULL;
        }
    }

    return (void*)(virt_base + page_offset);
}

int vmm_map_mmio(uint32_t phys_addr, uint32_t size) {
    return vmm_ioremap(phys_addr, size) ? 0 : -1;
}

int vmm_validate_user_range(page_directory_t *pd, uint32_t addr, uint32_t len, int write_required) {
    if (len == 0) return 1;
    if (addr < DIONNEX_USER_VADDR_MIN) return 0;

    uint64_t end = (uint64_t)addr + (uint64_t)len;
    if (end > DIONNEX_USER_VADDR_MAX || end < addr) return 0;

    if (!pd) pd = current_directory ? current_directory : kernel_directory;

    uint32_t page_start = addr & PAGE_ADDR_MASK;
    uint32_t page_end = (uint32_t)ALIGN_UP(end, PAGE_SIZE);

    for (uint32_t page = page_start; page < page_end; page += PAGE_SIZE) {
        uint32_t flags = vmm_get_flags_in(pd, page);
        if (!(flags & VMM_PRESENT) || !(flags & VMM_USER)) {
            return 0;
        }
        if (write_required && !(flags & VMM_WRITABLE)) {
            return 0;
        }
    }
    return 1;
}

int vmm_validate_user_string(page_directory_t *pd, const char *str, uint32_t max_len) {
    if (!str || max_len == 0) return 0;
    uint32_t addr = (uint32_t)(uintptr_t)str;
    if (addr < DIONNEX_USER_VADDR_MIN || addr >= DIONNEX_USER_VADDR_MAX) return 0;

    for (uint32_t i = 0; i < max_len; i++) {
        uint32_t cur = addr + i;
        if (cur >= DIONNEX_USER_VADDR_MAX) return 0;
        if ((i == 0) || ((cur & 0xFFFU) == 0)) {
            uint32_t flags = vmm_get_flags_in(pd, cur);
            if (!(flags & VMM_PRESENT) || !(flags & VMM_USER)) {
                return 0;
            }
        }
        if (str[i] == '\0') {
            return 1;
        }
    }
    return 0;
}
