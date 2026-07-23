/* ============================================================================
 * Dionnex Monolithic Kernel - ELF32 Executable Loader Header
 * ============================================================================
 */
#ifndef _DIONNEX_ELF_H_
#define _DIONNEX_ELF_H_

#include <types.h>

#define ELF_MAGIC 0x464C457F // "\x7FELF"

struct elf32_header {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed));

typedef struct elf32_header elf32_header_t;

struct elf32_phdr {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed));

typedef struct elf32_phdr elf32_phdr_t;

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6

void elf_init(void);
int elf_validate(const uint8_t *data);
int elf_load(const uint8_t *data);
int elf_load_file(const char *path);

#endif /* _DIONNEX_ELF_H_ */
