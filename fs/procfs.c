/* ============================================================================
 * Dionnex Monolithic Kernel - Process Filesystem (PROCFS) Implementation
 * ============================================================================
 */
#include <fs/procfs.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <kernel/cpuid.h>
#include <kernel/klog.h>
#include <drivers/pci.h>
#include <drivers/ata.h>
#include <string.h>

#define PROC_NUM_FILES 10

static vfs_node_t proc_root_node;
static vfs_node_t proc_file_nodes[PROC_NUM_FILES];

static const char *proc_file_names[PROC_NUM_FILES] = {
    "version",
    "uptime",
    "meminfo",
    "cpuinfo",
    "devices",
    "modules",
    "tasks",
    "pci",
    "disk",
    "klog"
};

static int procfs_generate_content(const char *name, char *buf, uint32_t max_len) {
    if (!name || !buf || max_len == 0) return 0;
    buf[0] = '\0';
    char tmp[128];

    if (strcmp(name, "version") == 0) {
        strcat(buf, "Dionnex Monolithic Kernel v0.1.0 (Genesis)\nx86 32-bit Monolithic\nGPLv2\n");
    } else if (strcmp(name, "uptime") == 0) {
        uint32_t secs = timer_get_ticks() / 1000;
        utoa(secs, tmp, 10);
        strcat(buf, tmp);
        strcat(buf, " seconds\n");
    } else if (strcmp(name, "meminfo") == 0) {
        uint32_t total_p = pmm_get_total_count();
        uint32_t free_p  = pmm_get_free_count();
        strcat(buf, "MemTotal:  "); utoa(total_p * 4, tmp, 10); strcat(buf, tmp); strcat(buf, " KB\n");
        strcat(buf, "MemFree:   "); utoa(free_p * 4, tmp, 10);  strcat(buf, tmp); strcat(buf, " KB\n");
        strcat(buf, "MemUsed:   "); utoa((total_p - free_p) * 4, tmp, 10); strcat(buf, tmp); strcat(buf, " KB\n");
        strcat(buf, "HeapFree:  "); utoa(heap_get_free() / 1024, tmp, 10); strcat(buf, tmp); strcat(buf, " KB\n");
        strcat(buf, "HeapUsed:  "); utoa(heap_get_used() / 1024, tmp, 10); strcat(buf, tmp); strcat(buf, " KB\n");
    } else if (strcmp(name, "cpuinfo") == 0) {
        char vendor[13];
        char brand[49];
        cpuid_get_vendor(vendor);
        cpuid_get_brand(brand);
        strcat(buf, "Vendor:   "); strcat(buf, vendor); strcat(buf, "\n");
        strcat(buf, "Model:    "); strcat(buf, brand); strcat(buf, "\n");
        strcat(buf, "Arch:     x86 32-bit\n");
    } else if (strcmp(name, "devices") == 0) {
        strcat(buf, "Character Devices:\n");
        strcat(buf, "  1 tty/vga\n  2 keyboard\n  3 serial/com1\n  4 rtc\n\n");
        strcat(buf, "Block Devices:\n");
        strcat(buf, "  1 ata/primary-master\n  2 ata/primary-slave\n");
    } else if (strcmp(name, "modules") == 0) {
        strcat(buf, "Registered Kernel Modules:\n");
        strcat(buf, "  pmm      v0.1.0 [loaded]\n");
        strcat(buf, "  vmm      v0.1.0 [loaded]\n");
        strcat(buf, "  heap     v0.1.0 [loaded]\n");
        strcat(buf, "  sched    v0.1.0 [loaded]\n");
        strcat(buf, "  vfs      v0.1.0 [loaded]\n");
        strcat(buf, "  procfs   v0.1.0 [loaded]\n");
    } else if (strcmp(name, "tasks") == 0) {
        strcat(buf, "PID  NAME             STATE\n");
        strcat(buf, "0    kernel_idle      RUNNING\n");
    } else if (strcmp(name, "pci") == 0) {
        strcat(buf, "PCI Devices Count: ");
        utoa(pci_get_count(), tmp, 10);
        strcat(buf, tmp);
        strcat(buf, "\n");
    } else if (strcmp(name, "disk") == 0) {
        strcat(buf, "ATA Hard Disk Drives: ");
        utoa(ata_get_drive_count(), tmp, 10);
        strcat(buf, tmp);
        strcat(buf, "\n");
    } else if (strcmp(name, "klog") == 0) {
        klog_read_bytes(buf, max_len - 1);
    }

    return strlen(buf);
}

static int procfs_vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buf) {
    if (!node || !buf) return -1;

    char *dyn_buf = (char*)kmalloc(2048);
    if (!dyn_buf) return -1;

    int total_len = procfs_generate_content(node->name, dyn_buf, 2048);
    if (offset >= (uint32_t)total_len) {
        kfree(dyn_buf);
        return 0;
    }

    uint32_t to_read = size;
    if (offset + to_read > (uint32_t)total_len) {
        to_read = total_len - offset;
    }

    memcpy(buf, dyn_buf + offset, to_read);
    kfree(dyn_buf);
    return (int)to_read;
}

static vfs_node_t *procfs_vfs_readdir(vfs_node_t *node, uint32_t index) {
    (void)node;
    if (index < PROC_NUM_FILES) {
        return &proc_file_nodes[index];
    }
    return NULL;
}

static vfs_node_t *procfs_vfs_finddir(vfs_node_t *node, const char *name) {
    (void)node;
    if (!name) return NULL;
    for (int i = 0; i < PROC_NUM_FILES; i++) {
        if (strcmp(proc_file_names[i], name) == 0) {
            return &proc_file_nodes[i];
        }
    }
    return NULL;
}

static vfs_ops_t procfs_vfs_ops = {
    .read = procfs_vfs_read,
    .write = NULL,
    .open = NULL,
    .close = NULL,
    .readdir = procfs_vfs_readdir,
    .finddir = procfs_vfs_finddir,
    .create = NULL,
    .delete = NULL
};

vfs_node_t *procfs_get_root_node(void) {
    return &proc_root_node;
}

void procfs_init(void) {
    memset(&proc_root_node, 0, sizeof(vfs_node_t));
    strcpy(proc_root_node.name, "proc");
    proc_root_node.flags = VFS_DIR;
    proc_root_node.ops = &procfs_vfs_ops;

    for (int i = 0; i < PROC_NUM_FILES; i++) {
        memset(&proc_file_nodes[i], 0, sizeof(vfs_node_t));
        strncpy(proc_file_nodes[i].name, proc_file_names[i], 127);
        proc_file_nodes[i].flags = VFS_FILE;
        proc_file_nodes[i].inode = 100 + i;
        proc_file_nodes[i].ops = &procfs_vfs_ops;
    }

    printk("Procfs: /proc mounted\n");
}
