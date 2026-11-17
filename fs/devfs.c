/* ============================================================================
 * Dionnex Monolithic Kernel - Device Filesystem (/dev) Implementation
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <fs/devfs.h>
#include <kernel/printk.h>
#include <kernel/klog.h>
#include <drivers/vga.h>
#include <drivers/serial.h>
#include <mm/heap.h>
#include <string.h>

#define DEVFS_MAX_NODES 8

static vfs_node_t devfs_root;
static vfs_node_t devfs_nodes[DEVFS_MAX_NODES];
static uint32_t devfs_count = 0;

static int dev_null_read(vfs_node_t *n, uint32_t off, uint32_t sz, uint8_t *buf) {
    (void)n; (void)off; (void)sz; (void)buf;
    return 0;
}

static int dev_null_write(vfs_node_t *n, uint32_t off, uint32_t sz, const uint8_t *buf) {
    (void)n; (void)off; (void)buf;
    return (int)sz;
}

static int dev_zero_read(vfs_node_t *n, uint32_t off, uint32_t sz, uint8_t *buf) {
    (void)n; (void)off;
    if (!buf) return -1;
    memset(buf, 0, sz);
    return (int)sz;
}

static int dev_tty_write(vfs_node_t *n, uint32_t off, uint32_t sz, const uint8_t *buf) {
    (void)n; (void)off;
    if (!buf) return -1;
    for (uint32_t i = 0; i < sz; i++) {
        vga_putchar((char)buf[i]);
        if (buf[i] == '\n') serial_putchar('\r');
        serial_putchar((char)buf[i]);
    }
    return (int)sz;
}

static int dev_klog_read(vfs_node_t *n, uint32_t off, uint32_t sz, uint8_t *buf) {
    (void)n; (void)off;
    if (!buf || sz == 0) return 0;
    return klog_read_bytes((char*)buf, sz);
}

static vfs_node_t *devfs_readdir(vfs_node_t *dir, uint32_t index) {
    (void)dir;
    if (index < devfs_count) return &devfs_nodes[index];
    return NULL;
}

static vfs_node_t *devfs_finddir(vfs_node_t *dir, const char *name) {
    (void)dir;
    if (!name) return NULL;
    for (uint32_t i = 0; i < devfs_count; i++) {
        if (strcmp(devfs_nodes[i].name, name) == 0) {
            return &devfs_nodes[i];
        }
    }
    return NULL;
}

static vfs_ops_t devfs_dir_ops = {
    .readdir = devfs_readdir,
    .finddir = devfs_finddir
};

static vfs_ops_t ops_null = { .read = dev_null_read, .write = dev_null_write };
static vfs_ops_t ops_zero = { .read = dev_zero_read, .write = dev_null_write };
static vfs_ops_t ops_tty  = { .read = dev_null_read, .write = dev_tty_write };
static vfs_ops_t ops_klog = { .read = dev_klog_read, .write = dev_null_write };

static void devfs_add_node(const char *name, vfs_ops_t *ops) {
    if (devfs_count >= DEVFS_MAX_NODES) return;
    vfs_node_t *node = &devfs_nodes[devfs_count];
    memset(node, 0, sizeof(vfs_node_t));
    strncpy(node->name, name, 127);
    node->flags = VFS_CHARDEV;
    node->inode = devfs_count + 1U;
    node->parent = &devfs_root;
    node->ops = ops;
    devfs_count++;
}

void devfs_init(void) {
    memset(&devfs_root, 0, sizeof(devfs_root));
    strcpy(devfs_root.name, "dev");
    devfs_root.flags = VFS_DIR;
    devfs_root.ops = &devfs_dir_ops;
    devfs_count = 0;

    devfs_add_node("null", &ops_null);
    devfs_add_node("zero", &ops_zero);
    devfs_add_node("tty",  &ops_tty);
    devfs_add_node("klog", &ops_klog);

    printk("Devfs: /dev mounted (%u character devices)\n", devfs_count);
}

vfs_node_t *devfs_get_root_node(void) {
    return &devfs_root;
}
