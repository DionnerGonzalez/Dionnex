/* ============================================================================
 * Dionnex Monolithic Kernel - ELF32 Executable Loader Implementation
 * ============================================================================
 */
#include <kernel/elf.h>
#include <kernel/printk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/task.h>
#include <fs/vfs.h>
#include <string.h>

void elf_init(void) {
    printk("ELF: ELF32 loader initialized\n");
}

int elf_validate(const uint8_t *data) {
    if (!data) return 0;
    const elf32_header_t *hdr = (const elf32_header_t*)data;

    if (*(uint32_t*)hdr->e_ident != ELF_MAGIC) return 0;
    if (hdr->e_ident[4] != 1) return 0; // 32-bit
    if (hdr->e_ident[5] != 1) return 0; // Little Endian
    if (hdr->e_machine != 3)  return 0; // EM_3386 (x86)

    return 1;
}

int elf_load(const uint8_t *data) {
    if (!elf_validate(data)) {
        printk("ELF: Invalid ELF32 binary header\n");
        return -1;
    }

    const elf32_header_t *hdr = (const elf32_header_t*)data;
    const elf32_phdr_t *phdr = (const elf32_phdr_t*)(data + hdr->e_phoff);

    for (uint16_t i = 0; i < hdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            uint32_t virt_start = phdr[i].p_vaddr;
            uint32_t mem_size   = phdr[i].p_memsz;
            uint32_t file_size  = phdr[i].p_filesz;
            uint32_t offset     = phdr[i].p_offset;

            uint32_t page_count = (mem_size + PAGE_SIZE - 1) / PAGE_SIZE;
            for (uint32_t p = 0; p < page_count; p++) {
                uint32_t phys = pmm_alloc_page();
                vmm_map_page(virt_start + (p * PAGE_SIZE), phys, VMM_PRESENT | VMM_WRITABLE | VMM_USER);
            }

            // Copy file payload
            memcpy((void*)virt_start, data + offset, file_size);

            // Zero remaining segment memory
            if (mem_size > file_size) {
                memset((void*)(virt_start + file_size), 0, mem_size - file_size);
            }
        }
    }

    task_t *t = task_create("elf_proc", (void(*)(void))hdr->e_entry, 5);
    if (!t) return -1;

    return (int)t->pid;
}

int elf_load_file(const char *path) {
    vfs_node_t *node = vfs_open(path);
    if (!node) {
        printk("ELF: File not found: %s\n", path);
        return -1;
    }

    if (node->size == 0) {
        printk("ELF: File is empty: %s\n", path);
        vfs_close(node);
        return -1;
    }

    uint8_t *file_buf = (uint8_t*)kmalloc(node->size);
    if (!file_buf) {
        printk("ELF: Failed to allocate buffer for file loading\n");
        vfs_close(node);
        return -1;
    }

    int bytes = vfs_read(node, 0, node->size, file_buf);
    vfs_close(node);

    if (bytes <= 0) {
        kfree(file_buf);
        return -1;
    }

    int pid = elf_load(file_buf);
    kfree(file_buf);
    return pid;
}
