/* ============================================================================
 * Dionnex Monolithic Kernel - ELF32 Executable Loader Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/elf_loader.h>
#include <kernel/printk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <string.h>

int elf_validate(uint8_t *data, uint32_t size) {
    if (!data || size < sizeof(elf32_header_t)) return 0;

    elf32_header_t *hdr = (elf32_header_t*)data;
    uint32_t magic = *(uint32_t*)hdr->e_ident;

    if (magic != ELF_MAGIC) return 0;
    if (hdr->e_ident[4] != 1) return 0; // Must be 32-bit ELF
    if (hdr->e_ident[5] != 1) return 0; // Must be Little-Endian
    if (hdr->e_type != 2 && hdr->e_type != 3) return 0; // ET_EXEC or ET_DYN

    return 1;
}

uint32_t elf_get_entry(uint8_t *data) {
    if (!elf_validate(data, sizeof(elf32_header_t))) return 0;
    elf32_header_t *hdr = (elf32_header_t*)data;
    return hdr->e_entry;
}

int elf_load_segments(uint8_t *data, uint32_t size, uint32_t *page_dir) {
    (void)page_dir; // Paging operates on current address space or active directory
    if (!elf_validate(data, size)) return -1;

    elf32_header_t *hdr = (elf32_header_t*)data;
    elf32_phdr_t *ph = (elf32_phdr_t*)(data + hdr->e_phoff);
    uint32_t count = 0;

    for (uint16_t i = 0; i < hdr->e_phnum; i++, ph++) {
        if (ph->p_type != PT_LOAD) continue;

        uint32_t vaddr = ph->p_vaddr;
        uint32_t memsz = ph->p_memsz;
        uint32_t filesz = ph->p_filesz;
        uint32_t offset = ph->p_offset;

        uint32_t page_start = vaddr & ~0xFFF;
        uint32_t page_end = (vaddr + memsz + 0xFFF) & ~0xFFF;

        for (uint32_t page = page_start; page < page_end; page += 4096) {
            void *phys = pmm_alloc_page();
            if (!phys) {
                printk("ELF: Memory allocation failed during segment load\n");
                return -1;
            }
            uint32_t flags = VMM_PRESENT | VMM_USER;
            if (ph->p_flags & PF_W) flags |= VMM_WRITABLE;
            vmm_map_page(page, (uint32_t)phys, flags);
        }

        // Copy segment content and clear remaining BSS memory
        memset((void*)vaddr, 0, memsz);
        if (filesz > 0 && (offset + filesz <= size)) {
            memcpy((void*)vaddr, data + offset, filesz);
        }

        count++;
    }

    printk("ELF: Loaded %u PT_LOAD segments, entry point at 0x%x\n", count, hdr->e_entry);
    return 0;
}
