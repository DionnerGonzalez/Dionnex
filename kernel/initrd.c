/* ============================================================================
 * Dionnex Monolithic Kernel - Init RAM Disk (InitRD) Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/initrd.h>
#include <kernel/printk.h>
#include <string.h>

extern uint8_t _binary_init_elf_start[];
extern uint8_t _binary_init_elf_end[];
extern uint8_t _binary_shell_user_elf_start[];
extern uint8_t _binary_shell_user_elf_end[];

#define MAX_INITRD_FILES 16

static initrd_file_t files[MAX_INITRD_FILES];
static uint32_t file_count = 0;

void initrd_init(void) {
    file_count = 0;

    uint32_t init_sz = (uint32_t)(_binary_init_elf_end - _binary_init_elf_start);
    if (init_sz > 0) {
        strncpy(files[file_count].name, "init.elf", 63);
        files[file_count].data = _binary_init_elf_start;
        files[file_count].size = init_sz;
        file_count++;
    }

    uint32_t sh_sz = (uint32_t)(_binary_shell_user_elf_end - _binary_shell_user_elf_start);
    if (sh_sz > 0) {
        strncpy(files[file_count].name, "shell_user.elf", 63);
        files[file_count].data = _binary_shell_user_elf_start;
        files[file_count].size = sh_sz;
        file_count++;
    }

    printk("Initrd: Embedded RAM disk loaded with %u binary files\n", file_count);
}

uint8_t *initrd_get_file(const char *name, uint32_t *size) {
    if (!name) return NULL;

    for (uint32_t i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, name) == 0) {
            if (size) *size = files[i].size;
            return files[i].data;
        }
    }

    if (size) *size = 0;
    return NULL;
}

void initrd_list(void) {
    printk("INITRD EMBEDDED BINARIES:\n");
    printk("----------------------------------------\n");
    for (uint32_t i = 0; i < file_count; i++) {
        printk("  %-20s %u bytes (Address: 0x%x)\n", files[i].name, files[i].size, (uint32_t)files[i].data);
    }
}
