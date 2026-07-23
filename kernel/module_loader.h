/* ============================================================================
 * Dionnex Monolithic Kernel - Module Loader Header
 * Author: Ihosvanni Dionner Gonzalez
 * Adapted from Linux kernel (GPLv2)
 * ============================================================================
 */
#ifndef _DIONNEX_MODULE_LOADER_H_
#define _DIONNEX_MODULE_LOADER_H_

#include <types.h>

#define ET_REL 1

#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_NOBITS   8
#define SHT_REL      9

#define SHN_UNDEF    0

#define ELF32_R_SYM(i)  ((i) >> 8)
#define ELF32_R_TYPE(i) ((uint8_t)(i))

#define R_386_32   1
#define R_386_PC32 2
#define R_386_16   4
#define R_386_8    5

#define SHF_ALLOC  0x2

typedef struct {
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
} elf32_ehdr;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} elf32_shdr;

typedef struct {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint8_t  st_info;
    uint8_t  st_other;
    uint16_t st_shndx;
} elf32_sym;

typedef struct {
    uint32_t r_offset;
    uint32_t r_info;
} elf32_rel;

struct kernel_module_struct {
    char name[64];
    uint32_t version;
    uint8_t loaded;
    uint32_t base_addr;
    uint32_t total_size;
    void (*init_func)(void);
    void (*cleanup_func)(void);
    struct kernel_module_struct *next;
};

typedef struct kernel_module_struct kernel_module_t;

int module_load_binary(const char *name, uint8_t *data, uint32_t size);
int module_unload_binary(const char *name);
void module_list_loaded(void);
uint32_t module_get_loaded_count(void);
int module_get_proc_info(char *buf, uint32_t max_len);
void module_loader_init(void);

#endif /* _DIONNEX_MODULE_LOADER_H_ */
