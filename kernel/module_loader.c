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

static int module_validate_elf(const uint8_t *data, uint32_t size) {
    if (!data || size < sizeof(elf32_ehdr)) return -1;

    const elf32_ehdr *hdr = (const elf32_ehdr*)data;

    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L'  || hdr->e_ident[3] != 'F') {
        return -1;
    }
    if (hdr->e_ident[4] != 1 || hdr->e_ident[5] != 1) return -1;
    if (hdr->e_type != ET_REL || hdr->e_machine != EM_386) return -1;
    if (hdr->e_shoff == 0 || hdr->e_shnum == 0) return -1;
    if (hdr->e_shentsize < sizeof(elf32_shdr)) return -1;

    uint64_t sh_end = (uint64_t)hdr->e_shoff + ((uint64_t)hdr->e_shnum * hdr->e_shentsize);
    if (hdr->e_shoff > size || sh_end > size) return -1;

    const elf32_shdr *shdrs = (const elf32_shdr*)(data + hdr->e_shoff);
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_type != SHT_NOBITS && shdrs[i].sh_size > 0) {
            uint64_t sec_end = (uint64_t)shdrs[i].sh_offset + shdrs[i].sh_size;
            if (shdrs[i].sh_offset > size || sec_end > size) {
                return -1;
            }
        }
    }

    return 0;
}

static void module_parse_modinfo(const uint8_t *data, uint32_t size,
                                 const elf32_ehdr *hdr, const elf32_shdr *shdrs,
                                 kernel_module_t *mod) {
    if (hdr->e_shstrndx >= hdr->e_shnum) return;
    const elf32_shdr *shstr_sh = &shdrs[hdr->e_shstrndx];
    if (shstr_sh->sh_offset >= size || shstr_sh->sh_offset + shstr_sh->sh_size > size) return;
    const char *shstr = (const char*)(data + shstr_sh->sh_offset);

    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_name >= shstr_sh->sh_size) continue;
        const char *sec_name = shstr + shdrs[i].sh_name;
        if (strcmp(sec_name, ".modinfo") == 0 && shdrs[i].sh_size > 0) {
            const char *p = (const char*)(data + shdrs[i].sh_offset);
            uint32_t rem = shdrs[i].sh_size;
            while (rem > 0) {
                uint32_t len = 0;
                while (len < rem && p[len] != '\0') len++;
                if (len > 0 && len < rem) {
                    if (strncmp(p, "author=", 7) == 0) {
                        strncpy(mod->author, p + 7, sizeof(mod->author) - 1);
                    } else if (strncmp(p, "description=", 12) == 0) {
                        strncpy(mod->description, p + 12, sizeof(mod->description) - 1);
                    } else if (strncmp(p, "license=", 8) == 0) {
                        strncpy(mod->license, p + 8, sizeof(mod->license) - 1);
                    } else if (strncmp(p, "version=", 8) == 0) {
                        strncpy(mod->version, p + 8, sizeof(mod->version) - 1);
                    } else if (strncmp(p, "depends=", 8) == 0) {
                        strncpy(mod->depends, p + 8, sizeof(mod->depends) - 1);
                    }
                }
                if (len >= rem) break;
                p += len + 1;
                rem -= len + 1;
            }
            break;
        }
    }
}

kernel_module_t *module_find_loaded(const char *name) {
    if (!name) return NULL;
    kernel_module_t *curr = module_list_head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

int module_load_binary(const char *name, const uint8_t *data, uint32_t size) {
    if (!name || !data || size == 0) return -1;

    if (module_find_loaded(name)) {
        printk("Module: '%s' is already loaded\n", name);
        return -1;
    }

    if (module_validate_elf(data, size) != 0) {
        printk("Module: Invalid or corrupted ELF32 ET_REL module '%s'\n", name);
        return -1;
    }

    const elf32_ehdr *hdr = (const elf32_ehdr*)data;
    const elf32_shdr *shdrs = (const elf32_shdr*)(data + hdr->e_shoff);

    /* Allocate temporary non-mutating tables (so .rodata initrd image stays untouched!) */
    uint32_t *sec_addrs = (uint32_t*)kcalloc(hdr->e_shnum, sizeof(uint32_t));
    if (!sec_addrs) return -1;

    /* Compute required size respecting section alignment */
    uint32_t total_size = 0;
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_flags & SHF_ALLOC) {
            uint32_t align = shdrs[i].sh_addralign;
            if (align < 4U) align = 4U;
            if ((align & (align - 1U)) != 0) align = 4U;
            total_size = ALIGN_UP(total_size, align);
            total_size += shdrs[i].sh_size;
        }
    }
    if (total_size == 0) total_size = 1024;
    total_size = ALIGN_UP(total_size, 16U);

    uint32_t base = (uint32_t)kmalloc_aligned(total_size, 16U);
    if (!base) {
        kfree(sec_addrs);
        printk("Module: Out of memory loading '%s'\n", name);
        return -1;
    }
    memset((void*)base, 0, total_size);

    /* Copy SHF_ALLOC sections into allocated module memory */
    uint32_t cur_off = 0;
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_flags & SHF_ALLOC) {
            uint32_t align = shdrs[i].sh_addralign;
            if (align < 4U) align = 4U;
            if ((align & (align - 1U)) != 0) align = 4U;
            cur_off = ALIGN_UP(cur_off, align);

            sec_addrs[i] = base + cur_off;
            if (shdrs[i].sh_type != SHT_NOBITS && shdrs[i].sh_size > 0) {
                memcpy((void*)sec_addrs[i], data + shdrs[i].sh_offset, shdrs[i].sh_size);
            }
            cur_off += shdrs[i].sh_size;
        }
    }

    /* Locate .symtab and .strtab */
    const elf32_shdr *symtab_shdr = NULL;
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            symtab_shdr = &shdrs[i];
            break;
        }
    }

    if (!symtab_shdr || symtab_shdr->sh_link >= hdr->e_shnum || symtab_shdr->sh_size < sizeof(elf32_sym)) {
        printk("Module: '%s' missing valid symbol table\n", name);
        kfree((void*)base);
        kfree(sec_addrs);
        return -1;
    }

    const elf32_shdr *strtab_shdr = &shdrs[symtab_shdr->sh_link];
    const elf32_sym *syms = (const elf32_sym*)(data + symtab_shdr->sh_offset);
    const char *strtab = (const char*)(data + strtab_shdr->sh_offset);
    uint32_t sym_count = symtab_shdr->sh_size / sizeof(elf32_sym);

    uint32_t *sym_vals = (uint32_t*)kcalloc(sym_count, sizeof(uint32_t));
    if (!sym_vals) {
        kfree((void*)base);
        kfree(sec_addrs);
        return -1;
    }

    /* Resolve internal and external symbols into temporary sym_vals[] */
    for (uint32_t i = 0; i < sym_count; i++) {
        if (syms[i].st_shndx == SHN_ABS) {
            sym_vals[i] = syms[i].st_value;
        } else if (syms[i].st_shndx != SHN_UNDEF && syms[i].st_shndx < hdr->e_shnum) {
            sym_vals[i] = sec_addrs[syms[i].st_shndx] + syms[i].st_value;
        } else if (syms[i].st_shndx == SHN_UNDEF && syms[i].st_name != 0) {
            if (syms[i].st_name >= strtab_shdr->sh_size) {
                kfree(sym_vals); kfree((void*)base); kfree(sec_addrs);
                return -1;
            }
            const char *sym_name = strtab + syms[i].st_name;
            uint32_t resolved = 0;
            if (module_resolve_symbol(sym_name, &resolved) != 0) {
                printk("Module: '%s' undefined symbol '%s'\n", name, sym_name);
                kfree(sym_vals); kfree((void*)base); kfree(sec_addrs);
                return -1;
            }
            sym_vals[i] = resolved;
        }
    }

    /* Apply x86 relocations with strict bounds checks */
    for (uint16_t i = 0; i < hdr->e_shnum; i++) {
        if (shdrs[i].sh_type != SHT_REL) continue;

        uint32_t target_sec = shdrs[i].sh_info;
        if (target_sec >= hdr->e_shnum || !(shdrs[target_sec].sh_flags & SHF_ALLOC)) {
            continue;
        }

        const elf32_rel *rels = (const elf32_rel*)(data + shdrs[i].sh_offset);
        uint32_t rel_count = shdrs[i].sh_size / sizeof(elf32_rel);
        uint32_t target_base = sec_addrs[target_sec];
        uint32_t target_size = shdrs[target_sec].sh_size;

        for (uint32_t r = 0; r < rel_count; r++) {
            uint32_t sym_idx = ELF32_R_SYM(rels[r].r_info);
            uint32_t type    = ELF32_R_TYPE(rels[r].r_info);

            if (sym_idx >= sym_count || rels[r].r_offset + 4U > target_size) {
                printk("Module: '%s' out-of-bounds relocation\n", name);
                kfree(sym_vals); kfree((void*)base); kfree(sec_addrs);
                return -1;
            }

            uint32_t S = sym_vals[sym_idx];
            uint32_t P = target_base + rels[r].r_offset;
            uint32_t *loc = (uint32_t*)P;

            switch (type) {
                case R_386_NONE:
                    break;
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
                    printk("Module: '%s' unsupported relocation type %u\n", name, type);
                    kfree(sym_vals); kfree((void*)base); kfree(sec_addrs);
                    return -1;
            }
        }
    }

    /* Find module_init and module_cleanup entry points */
    int (*init_func)(void) = NULL;
    void (*cleanup_func)(void) = NULL;

    for (uint32_t i = 0; i < sym_count; i++) {
        if (syms[i].st_name != 0 && syms[i].st_name < strtab_shdr->sh_size) {
            const char *sym_name = strtab + syms[i].st_name;
            if (strcmp(sym_name, "module_init") == 0) {
                init_func = (int (*)(void))sym_vals[i];
            } else if (strcmp(sym_name, "module_cleanup") == 0) {
                cleanup_func = (void (*)(void))sym_vals[i];
            }
        }
    }

    kfree(sym_vals);
    kfree(sec_addrs);

    if (!init_func) {
        printk("Module: '%s' has no module_init symbol\n", name);
        kfree((void*)base);
        return -1;
    }

    kernel_module_t *mod = (kernel_module_t*)kcalloc(1, sizeof(kernel_module_t));
    if (!mod) {
        kfree((void*)base);
        return -1;
    }

    strncpy(mod->name, name, sizeof(mod->name) - 1);
    strcpy(mod->version, "0.1.0");
    strcpy(mod->author, "Dionnex");
    strcpy(mod->description, "Dionnex Kernel Module");
    strcpy(mod->license, "GPLv2");
    module_parse_modinfo(data, size, hdr, shdrs, mod);

    /* Verify module dependency if specified */
    kernel_module_t *dep = NULL;
    if (mod->depends[0] != '\0') {
        dep = module_find_loaded(mod->depends);
        if (!dep) {
            printk("Module: '%s' requires dependency '%s'\n", name, mod->depends);
            kfree(mod);
            kfree((void*)base);
            return -1;
        }
    }

    mod->loaded = 1;
    mod->base_addr = base;
    mod->total_size = total_size;
    mod->init_func = init_func;
    mod->cleanup_func = cleanup_func;

    printk("Module: Loading '%s' (%u bytes at 0x%x)\n", name, total_size, base);
    int init_rc = init_func();
    if (init_rc != 0) {
        printk("Module: '%s' init_func returned %d\n", name, init_rc);
        kfree(mod);
        kfree((void*)base);
        return -1;
    }

    if (dep) dep->refcount++;
    mod->next = module_list_head;
    module_list_head = mod;
    printk("Module: '%s' loaded successfully\n", name);
    return 0;
}

int module_unload_binary(const char *name) {
    if (!name) return -1;

    kernel_module_t *curr = module_list_head;
    kernel_module_t *prev = NULL;

    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            if (curr->refcount > 0) {
                printk("Module: Cannot unload '%s' (in use, refcount=%u)\n", name, curr->refcount);
                return -1;
            }
            if (curr->cleanup_func) {
                curr->cleanup_func();
            }
            if (curr->depends[0] != '\0') {
                kernel_module_t *dep = module_find_loaded(curr->depends);
                if (dep && dep->refcount > 0) dep->refcount--;
            }

            printk("Module: Unloading '%s'\n", name);
            if (prev) prev->next = curr->next;
            else module_list_head = curr->next;

            if (curr->base_addr) kfree((void*)curr->base_addr);
            kfree(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    printk("Module: '%s' not found\n", name);
    return -1;
}

void module_print_info(const char *name) {
    kernel_module_t *mod = module_find_loaded(name);
    if (!mod) {
        printk("modinfo: module '%s' not currently loaded\n", name ? name : "");
        return;
    }
    printk("filename:    %s.ko\n", mod->name);
    printk("version:     %s\n", mod->version);
    printk("license:     %s\n", mod->license);
    printk("author:      %s\n", mod->author);
    printk("description: %s\n", mod->description);
    printk("depends:     %s\n", mod->depends[0] ? mod->depends : "(none)");
    printk("refcount:    %u\n", mod->refcount);
    printk("addr/size:   0x%x (%u bytes)\n", mod->base_addr, mod->total_size);
}

void module_list_loaded(void) {
    printk("NAME                 SIZE     REFS  STATUS\n");
    printk("----                 ----     ----  ------\n");

    kernel_module_t *curr = module_list_head;
    uint32_t count = 0;
    while (curr) {
        printk("%-20s %-8u %-5u %s\n",
               curr->name, curr->total_size, curr->refcount,
               curr->loaded ? "loaded" : "unloaded");
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
        if (curr->loaded) count++;
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
        strcat(buf, " ");
        utoa(curr->refcount, tmp, 10);
        strcat(buf, tmp);
        strcat(buf, " - Live 0x");
        utoa(curr->base_addr, tmp, 16);
        strcat(buf, tmp);
        strcat(buf, "\n");
        curr = curr->next;
    }
    return (int)strlen(buf);
}
