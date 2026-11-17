/* ============================================================================
 * Dionnex Monolithic Kernel - Multiboot v1 Specification Header
 * ============================================================================
 */
#ifndef _DIONNEX_MULTIBOOT_H_
#define _DIONNEX_MULTIBOOT_H_

#include <types.h>

#define MULTIBOOT_HEADER_MAGIC      0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC  0x2BADB002

#define MULTIBOOT_INFO_MEMORY       (1 << 0)
#define MULTIBOOT_INFO_BOOTDEV      (1 << 1)
#define MULTIBOOT_INFO_CMDLINE      (1 << 2)
#define MULTIBOOT_INFO_MODS         (1 << 3)
#define MULTIBOOT_INFO_AOUT_SYMS    (1 << 4)
#define MULTIBOOT_INFO_ELF_SHDR     (1 << 5)
#define MULTIBOOT_INFO_MEM_MAP      (1 << 6)
#define MULTIBOOT_INFO_DRIVE_INFO   (1 << 7)
#define MULTIBOOT_INFO_CONFIG_TABLE (1 << 8)
#define MULTIBOOT_INFO_BOOT_LOADER  (1 << 9)
#define MULTIBOOT_INFO_APM_TABLE    (1 << 10)
#define MULTIBOOT_INFO_VBE_INFO     (1 << 11)
#define MULTIBOOT_INFO_FRAMEBUFFER  (1 << 12)

#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5

struct multiboot_mod_list {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t cmdline;
    uint32_t pad;
} __attribute__((packed));

typedef struct multiboot_mod_list multiboot_module_t;

#endif /* _DIONNEX_MULTIBOOT_H_ */
