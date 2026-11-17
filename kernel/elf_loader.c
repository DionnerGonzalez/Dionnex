/* ============================================================================
 * Dionnex Monolithic Kernel - ELF32 Executable Loader Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/elf_loader.h>
#include <kernel/printk.h>
#include <dionnex_rust.h>
#include <arch.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <string.h>

int elf_validate(const uint8_t *data, uint32_t size) {
    if (!dionnex_rust_validate_elf32(data, size)) return 0;

    const elf32_header_t *hdr = (const elf32_header_t*)data;

    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L'  || hdr->e_ident[3] != 'F') {
        return 0;
    }
    if (hdr->e_ident[4] != 1) return 0; /* ELFCLASS32 */
    if (hdr->e_ident[5] != 1) return 0; /* ELFDATA2LSB */
    if (hdr->e_type != ET_EXEC && hdr->e_type != ET_DYN) return 0;
    if (hdr->e_machine != EM_386) return 0;
    if (hdr->e_phnum == 0 || hdr->e_phentsize < sizeof(elf32_phdr_t)) return 0;

    uint64_t ph_end = (uint64_t)hdr->e_phoff + ((uint64_t)hdr->e_phnum * hdr->e_phentsize);
    if (ph_end > size || hdr->e_phoff > size) return 0;

    if (hdr->e_entry < DIONNEX_USER_VADDR_MIN || hdr->e_entry >= DIONNEX_USER_VADDR_MAX) {
        return 0;
    }

    return 1;
}

uint32_t elf_get_entry(const uint8_t *data, uint32_t size) {
    if (!elf_validate(data, size)) return 0;
    const elf32_header_t *hdr = (const elf32_header_t*)data;
    return hdr->e_entry;
}

int elf_load_segments(const uint8_t *data, uint32_t size, page_directory_t *page_dir) {
    if (!elf_validate(data, size) || !page_dir) return -1;

    const elf32_header_t *hdr = (const elf32_header_t*)data;
    uint32_t count = 0;

    for (uint16_t i = 0; i < hdr->e_phnum; i++) {
        const elf32_phdr_t *ph = (const elf32_phdr_t*)(data + hdr->e_phoff + ((uint32_t)i * hdr->e_phentsize));
        if (ph->p_type != PT_LOAD) continue;
        if (ph->p_memsz == 0) continue;

        /* Security: validate segment bounds and ensure strict user-space placement */
        if (ph->p_filesz > ph->p_memsz) return -1;
        if ((uint64_t)ph->p_offset + ph->p_filesz > size) return -1;
        if (ph->p_vaddr < DIONNEX_USER_VADDR_MIN) return -1;
        uint64_t seg_end = (uint64_t)ph->p_vaddr + ph->p_memsz;
        if (seg_end > DIONNEX_USER_VADDR_MAX || seg_end < ph->p_vaddr) return -1;

        uint32_t page_start = ph->p_vaddr & ~0xFFFU;
        uint32_t page_end   = (uint32_t)ALIGN_UP(seg_end, PAGE_SIZE);
        uint32_t flags      = VMM_PRESENT | VMM_USER;
        if (ph->p_flags & PF_W) flags |= VMM_WRITABLE;

        for (uint32_t page = page_start; page < page_end; page += PAGE_SIZE) {
            uint32_t phys = vmm_get_phys_in(page_dir, page) & ~0xFFFU;
            if (phys == 0) {
                phys = pmm_alloc_page();
                if (!phys) {
                    printk("ELF: Out of physical memory loading segment %u\n", i);
                    return -1;
                }
                vmm_zero_phys_page(phys);
                if (vmm_map_page_in(page_dir, page, phys, flags) != 0) {
                    pmm_free_page(phys);
                    return -1;
                }
            } else if (ph->p_flags & PF_W) {
                vmm_map_page_in(page_dir, page, phys, flags);
            }

            /* Copy intersection of [page, page + PAGE_SIZE) with [p_vaddr, p_vaddr + p_filesz) */
            uint32_t file_vstart = ph->p_vaddr;
            uint32_t file_vend   = ph->p_vaddr + ph->p_filesz;
            uint32_t copy_start  = MAX(page, file_vstart);
            uint32_t copy_end    = MIN(page + PAGE_SIZE, file_vend);

            if (copy_end > copy_start) {
                uint32_t page_off = copy_start - page;
                uint32_t file_off = ph->p_offset + (copy_start - file_vstart);
                uint32_t len      = copy_end - copy_start;
                vmm_write_phys_page(phys, page_off, data + file_off, len);
            }
        }

        count++;
    }

    if (count == 0) return -1;
    printk("ELF: Loaded %u PT_LOAD segments into isolated address space (entry 0x%x)\n",
           count, hdr->e_entry);
    return 0;
}
