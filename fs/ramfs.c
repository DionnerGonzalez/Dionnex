/* ============================================================================
 * Dionnex Monolithic Kernel - RAM File System (RAMFS) Implementation
 * ============================================================================
 */
#include <fs/ramfs.h>
#include <kernel/printk.h>
#include <string.h>

static ramfs_file_t ramfs_files[RAMFS_MAX_FILES];
static vfs_node_t ramfs_root_node;

static int ramfs_vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buf) {
    if (!node || !buf) return -1;
    ramfs_file_t *file = (ramfs_file_t*)node->private_data;
    if (!file || !file->used) return -1;

    if (offset >= file->size) return 0;
    uint32_t to_read = size;
    if (offset + to_read > file->size) {
        to_read = file->size - offset;
    }

    memcpy(buf, file->data + offset, to_read);
    return (int)to_read;
}

static int ramfs_vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buf) {
    if (!node || !buf) return -1;
    ramfs_file_t *file = (ramfs_file_t*)node->private_data;
    if (!file || !file->used) return -1;

    if (offset >= RAMFS_MAX_DATA) return -1;
    uint32_t to_write = size;
    if (offset + to_write > RAMFS_MAX_DATA) {
        to_write = RAMFS_MAX_DATA - offset;
    }

    memcpy(file->data + offset, buf, to_write);
    if (offset + to_write > file->size) {
        file->size = offset + to_write;
        node->size = file->size;
    }
    return (int)to_write;
}

static vfs_node_t *ramfs_vfs_readdir(vfs_node_t *node, uint32_t index) {
    (void)node;
    uint32_t active_idx = 0;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used) {
            if (active_idx == index) {
                return &ramfs_files[i].vfs_node;
            }
            active_idx++;
        }
    }
    return NULL;
}

static vfs_node_t *ramfs_vfs_finddir(vfs_node_t *node, const char *name) {
    (void)node;
    if (!name) return NULL;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used && strcmp(ramfs_files[i].name, name) == 0) {
            return &ramfs_files[i].vfs_node;
        }
    }
    return NULL;
}

static int ramfs_vfs_create(vfs_node_t *node, const char *name, uint32_t flags) {
    (void)node;
    (void)flags;
    return ramfs_create(name);
}

static int ramfs_vfs_delete(vfs_node_t *node, const char *name) {
    (void)node;
    return ramfs_delete(name);
}

static vfs_ops_t ramfs_vfs_ops = {
    .read = ramfs_vfs_read,
    .write = ramfs_vfs_write,
    .open = NULL,
    .close = NULL,
    .readdir = ramfs_vfs_readdir,
    .finddir = ramfs_vfs_finddir,
    .create = ramfs_vfs_create,
    .delete = ramfs_vfs_delete
};

int ramfs_find(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used && strcmp(ramfs_files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int ramfs_create(const char *name) {
    if (!name) return -1;
    if (ramfs_find(name) >= 0) return -1; // Exists

    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (!ramfs_files[i].used) {
            ramfs_files[i].used = 1;
            ramfs_files[i].size = 0;
            strncpy(ramfs_files[i].name, name, RAMFS_MAX_NAME - 1);
            ramfs_files[i].data[0] = '\0';

            // Init VFS Node
            memset(&ramfs_files[i].vfs_node, 0, sizeof(vfs_node_t));
            strncpy(ramfs_files[i].vfs_node.name, name, 127);
            ramfs_files[i].vfs_node.flags = VFS_FILE;
            ramfs_files[i].vfs_node.size = 0;
            ramfs_files[i].vfs_node.inode = i + 1;
            ramfs_files[i].vfs_node.private_data = &ramfs_files[i];
            ramfs_files[i].vfs_node.ops = &ramfs_vfs_ops;

            return 0;
        }
    }
    return -1;
}

int ramfs_delete(const char *name) {
    int idx = ramfs_find(name);
    if (idx < 0) return -1;

    ramfs_files[idx].used = 0;
    ramfs_files[idx].size = 0;
    ramfs_files[idx].name[0] = '\0';
    return 0;
}

int ramfs_read(const char *name, uint8_t *buf, uint32_t len) {
    int idx = ramfs_find(name);
    if (idx < 0 || !buf) return -1;

    uint32_t to_read = (len < ramfs_files[idx].size) ? len : ramfs_files[idx].size;
    memcpy(buf, ramfs_files[idx].data, to_read);
    return (int)to_read;
}

int ramfs_write(const char *name, const uint8_t *buf, uint32_t len) {
    int idx = ramfs_find(name);
    if (idx < 0 || !buf) return -1;

    uint32_t to_write = (len > RAMFS_MAX_DATA) ? RAMFS_MAX_DATA : len;
    memcpy(ramfs_files[idx].data, buf, to_write);
    ramfs_files[idx].size = to_write;
    ramfs_files[idx].vfs_node.size = to_write;
    return (int)to_write;
}

void ramfs_list(void) {
    printk("NAME                 SIZE\n");
    printk("----                 ----\n");

    int count = 0;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used) {
            printk("%-20s %u bytes\n", ramfs_files[i].name, ramfs_files[i].size);
            count++;
        }
    }
    if (count == 0) {
        printk("(empty)\n");
    }
}

uint32_t ramfs_get_file_count(void) {
    uint32_t count = 0;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used) count++;
    }
    return count;
}

vfs_node_t *ramfs_get_root_node(void) {
    return &ramfs_root_node;
}

void ramfs_init(void) {
    memset(ramfs_files, 0, sizeof(ramfs_files));

    memset(&ramfs_root_node, 0, sizeof(vfs_node_t));
    strcpy(ramfs_root_node.name, "ram");
    ramfs_root_node.flags = VFS_DIR;
    ramfs_root_node.ops = &ramfs_vfs_ops;

    ramfs_create("readme.txt");
    const char *welcome = "Welcome to Dionnex Monolithic Kernel v0.1!";
    ramfs_write("readme.txt", (const uint8_t*)welcome, strlen(welcome));

    printk("RAMFS: Initialized in-memory Virtual Filesystem (%d slots)\n", RAMFS_MAX_FILES);
}
