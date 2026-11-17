/* ============================================================================
 * Dionnex Monolithic Kernel - ELF32 Unified Loader Facade
 * ============================================================================
 */
#include <kernel/elf.h>
#include <kernel/elf_loader.h>
#include <kernel/process.h>
#include <kernel/scheduler_process.h>
#include <kernel/printk.h>
#include <mm/heap.h>
#include <fs/vfs.h>

void elf_init(void) {
    printk("ELF: ELF32 loader subsystem ready\n");
}

int elf_load(const uint8_t *data, uint32_t size) {
    process_t *proc = process_create("elf_proc", data, size);
    if (!proc) return -1;
    sched_proc_add(proc);
    return (int)proc->pid;
}

int elf_load_file(const char *path) {
    if (!path) return -1;
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
        printk("ELF: Failed to allocate %u byte buffer\n", node->size);
        vfs_close(node);
        return -1;
    }

    int bytes = vfs_read(node, 0, node->size, file_buf);
    vfs_close(node);

    if (bytes <= 0) {
        kfree(file_buf);
        return -1;
    }

    int pid = elf_load(file_buf, (uint32_t)bytes);
    kfree(file_buf);
    return pid;
}
