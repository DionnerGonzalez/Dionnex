/* ============================================================================
 * Dionnex Kernel - ELF Binary Executable Loader (process/elf.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

#define ELF_MAGIC 0x464C457F // "\x7FELF"

struct elf_header {
    uint32_t magic;
    uint8_t  e_ident[12];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t header_size;
    uint16_t ph_entry_size;
    uint16_t ph_num;
    uint16_t sh_entry_size;
    uint16_t sh_num;
    uint16_t strtab_index;
};

struct elf_program_header {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
};

int elf_validate(const void* data) {
    const struct elf_header* hdr = (const struct elf_header*)data;
    if (hdr->magic != ELF_MAGIC) {
        return -1; // Not a valid ELF binary
    }
    return 0;
}

uint32_t elf_load_binary(const void* data) {
    if (elf_validate(data) != 0) {
        return 0;
    }

    const struct elf_header* hdr = (const struct elf_header*)data;
    const uint8_t* base = (const uint8_t*)data;

    for (int i = 0; i < hdr->ph_num; i++) {
        const struct elf_program_header* ph = (const struct elf_program_header*)(base + hdr->phoff + (i * hdr->ph_entry_size));
        
        // PT_LOAD segment
        if (ph->type == 1) {
            void* dest = (void*)ph->vaddr;
            memset(dest, 0, ph->memsz);
            memcpy(dest, base + ph->offset, ph->filesz);
        }
    }

    return hdr->entry; // Address of entry point
}
