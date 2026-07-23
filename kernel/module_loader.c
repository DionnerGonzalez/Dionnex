/* ============================================================================
 * Dionnex Monolithic Kernel - Relocatable ELF Kernel Module Loader
 * Author: Ihosvanni Dionner Gonzalez
 * Adapted from Linux kernel: kernel/module/main.c (GPLv2)
 * ============================================================================
 */

#include <kernel/module_loader.h>
#include <kernel/module_symbols.h>
#include <kernel/printk.h>
#include <mm/heap.h>
#include <string.h>

static kernel_module_t *module_list_head = NULL;

void module_loader_init(void) {
    module_list_head = NULL;
    printk("Module Loader: ELF relocatable module support initialized\n");
}

static int module_validate_elf(uint8_t *data, uint32_t size) {
    if (!data || size < sizeof(elf32_ehdr)) {
        return -1;
    }

    elf32_ehdr *hdr = (elf32_ehdr*)data;

    // Magic check: \x7F ELF
    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L' || hdr->e_ident[3] != 'F') {
        return -1;
    }

    // 32-bit (1), Little endian (1)
    if (hdr->e_ident[4] != 1 || hdr->e_ident[5] != 1) {
        return -1;
    }

    // ET_REL (1)
    if (hdr->e_type != ET_REL) {
        return -1;
    }

    if (hdr->e_shoff == 0 || hdr->e_shnum == 0) {
        return -1;
    }

    return 0;
}

static uint32_t module_calc_size(elf32_ehdr *hdr, uint8_t *data) {
    elf32_shdr *shdrs = (elf32_shdr*)(data + hdr->e_shoff);
    uint32_t total_size = 0;

    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_flags & SHF_ALLOC) {
            uint32_t size = shdrs[i].sh_size;
            total_size += (size + 3) & ~3; // 4-byte align
        }
    }

    return total_size;
}

int module_load_binary(const char *name, uint8_t *data, uint32_t size) {
    if (!name || !data || size == 0) {
        return -1;
    }

    // Check if module is already loaded
    kernel_module_t *curr = module_list_head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            printk("Module: '%s' is already loaded\n", name);
            return -1;
        }
        curr = curr->next;
    }

    if (module_validate_elf(data, size) != 0) {
        printk("Module: Invalid ELF\n");
        return -1;
    }

    elf32_ehdr *hdr = (elf32_ehdr*)data;
    elf32_shdr *shdrs = (elf32_shdr*)(data + hdr->e_shoff);

    uint32_t total_size = module_calc_size(hdr, data);
    if (total_size == 0) {
        total_size = 1024; // minimum space
    }

    uint32_t base = (uint32_t)kmalloc(total_size);
    if (!base) {
        printk("Module: No memory\n");
        return -1;
    }

    memset((void*)base, 0, total_size);

    // Copy allocated sections into module base address
    uint32_t current_offset = 0;
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_flags & SHF_ALLOC) {
            shdrs[i].sh_addr = base + current_offset;
            if (shdrs[i].sh_type == SHT_NOBITS) {
                memset((void*)(base + current_offset), 0, shdrs[i].sh_size);
            } else if (shdrs[i].sh_size > 0) {
                memcpy((void*)(base + current_offset), data + shdrs[i].sh_offset, shdrs[i].sh_size);
            }
            current_offset += (shdrs[i].sh_size + 3) & ~3;
        }
    }

    // Locate .symtab and .strtab
    elf32_shdr *symtab_shdr = NULL;
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            symtab_shdr = &shdrs[i];
            break;
        }
    }

    if (!symtab_shdr || symtab_shdr->sh_link >= hdr->e_shnum) {
        printk("Module: No symbol table found\n");
        kfree((void*)base);
        return -1;
    }

    elf32_sym *syms = (elf32_sym*)(data + symtab_shdr->sh_offset);
    char *strtab = (char*)(data + shdrs[symtab_shdr->sh_link].sh_offset);
    uint32_t sym_count = symtab_shdr->sh_size / sizeof(elf32_sym);

    // First pass: update symbol values for internal symbols defined in sections
    for (uint32_t i = 0; i < sym_count; i++) {
        if (syms[i].st_shndx != SHN_UNDEF && syms[i].st_shndx < hdr->e_shnum) {
            syms[i].st_value = shdrs[syms[i].st_shndx].sh_addr + syms[i].st_value;
        }
    }

    // Resolve undefined symbols
    for (uint32_t i = 0; i < sym_count; i++) {
        if (syms[i].st_shndx == SHN_UNDEF && syms[i].st_name != 0) {
            char *sym_name = strtab + syms[i].st_name;
            uint32_t resolved_addr = 0;
            if (module_resolve_symbol(sym_name, &resolved_addr) != 0) {
                printk("Module: Undefined symbol '%s'\n", sym_name);
                kfree((void*)base);
                return -1;
            }
            syms[i].st_value = resolved_addr;
        }
    }

    // Apply relocations
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_type == SHT_REL) {
            elf32_rel *rels = (elf32_rel*)(data + shdrs[i].sh_offset);
            uint32_t rel_count = shdrs[i].sh_size / sizeof(elf32_rel);
            uint32_t target_section = shdrs[i].sh_info;

            if (target_section >= hdr->e_shnum) {
                continue;
            }

            uint32_t target_base = shdrs[target_section].sh_addr;

            for (uint32_t r = 0; r < rel_count; r++) {
                uint32_t sym_idx = ELF32_R_SYM(rels[r].r_info);
                uint32_t type = ELF32_R_TYPE(rels[r].r_info);

                if (sym_idx >= sym_count) {
                    continue;
                }

                uint32_t S = syms[sym_idx].st_value;
                uint32_t P = target_base + rels[r].r_offset;
                uint32_t *loc = (uint32_t*)P;

                switch (type) {
                    case R_386_32:
                        *loc += S;
                        break;
                    case R_386_PC32:
                        *loc += (S - P);
                        break;
                    case R_386_16:
                        *(uint16_t*)loc += (uint16_t)S;
                        break;
                    case R_386_8:
                        *(uint8_t*)loc += (uint8_t)S;
                        break;
                    default:
                        printk("Module: Unknown relocation type %u\n", type);
                        break;
                }
            }
        }
    }

    // Locate module_init and module_cleanup symbols
    void (*init_func)(void) = NULL;
    void (*cleanup_func)(void) = NULL;

    for (uint32_t i = 0; i < sym_count; i++) {
        if (syms[i].st_name != 0) {
            char *sym_name = strtab + syms[i].st_name;
            if (strcmp(sym_name, "module_init") == 0) {
                init_func = (void (*)(void))syms[i].st_value;
            } else if (strcmp(sym_name, "module_cleanup") == 0) {
                cleanup_func = (void (*)(void))syms[i].st_value;
            }
        }
    }

    if (!init_func) {
        printk("Module: No module_init found\n");
        kfree((void*)base);
        return -1;
    }

    // Register module
    kernel_module_t *mod = (kernel_module_t*)kmalloc(sizeof(kernel_module_t));
    if (!mod) {
        printk("Module: Failed to allocate module structure\n");
        kfree((void*)base);
        return -1;
    }

    memset(mod, 0, sizeof(kernel_module_t));
    strncpy(mod->name, name, 63);
    mod->version = 1;
    mod->loaded = 1;
    mod->base_addr = base;
    mod->total_size = total_size;
    mod->init_func = init_func;
    mod->cleanup_func = cleanup_func;
    mod->next = module_list_head;
    module_list_head = mod;

    printk("Module: Loading '%s' (%u bytes)\n", name, total_size);
    if (init_func) {
        init_func();
    }
    printk("Module: '%s' loaded successfully\n", name);

    return 0;
}

int module_unload_binary(const char *name) {
    if (!name) return -1;

    kernel_module_t *curr = module_list_head;
    kernel_module_t *prev = NULL;

    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            if (curr->cleanup_func) {
                curr->cleanup_func();
            }

            printk("Module: Unloading '%s'\n", name);

            if (prev) {
                prev->next = curr->next;
            } else {
                module_list_head = curr->next;
            }

            if (curr->base_addr) {
                kfree((void*)curr->base_addr);
            }

            kfree(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    printk("Module: '%s' not found\n", name);
    return -1;
}

void module_list_loaded(void) {
    printk("NAME                 SIZE     STATUS\n");
    printk("----                 ----     ------\n");

    kernel_module_t *curr = module_list_head;
    uint32_t count = 0;

    while (curr) {
        printk("%-20s %-8u %s\n", curr->name, curr->total_size, curr->loaded ? "loaded" : "unloaded");
        curr = curr->next;
        count++;
    }

    if (count == 0) {
        printk("(no modules loaded)\n");
    }
}

uint32_t module_get_loaded_count(void) {
    kernel_module_t *curr = module_list_head;
    uint32_t count = 0;
    while (curr) {
        if (curr->loaded) {
            count++;
        }
        curr = curr->next;
    }
    return count;
}

int module_get_proc_info(char *buf, uint32_t max_len) {
    if (!buf || max_len == 0) return 0;
    buf[0] = '\0';
    kernel_module_t *curr = module_list_head;
    char tmp[64];
    while (curr && strlen(buf) < max_len - 128) {
        strcat(buf, curr->name);
        strcat(buf, " ");
        utoa(curr->total_size, tmp, 10);
        strcat(buf, tmp);
        strcat(buf, " 0 - Live 0x");
        utoa(curr->base_addr, tmp, 16);
        strcat(buf, tmp);
        strcat(buf, "\n");
        curr = curr->next;
    }
    return strlen(buf);
}
