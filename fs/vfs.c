/* ============================================================================
 * Dionnex Monolithic Kernel - Virtual Filesystem (VFS) Implementation
 * ============================================================================
 */
#include <fs/vfs.h>
#include <mm/heap.h>
#include <kernel/printk.h>
#include <string.h>

static vfs_node_t *vfs_root = NULL;

static vfs_node_t *vfs_root_readdir(vfs_node_t *node, uint32_t index) {
    if (!node || !(node->flags & VFS_DIR)) return NULL;
    vfs_node_t *curr = node->children;
    uint32_t idx = 0;
    while (curr) {
        if (idx == index) return curr;
        idx++;
        curr = curr->next_sibling;
    }
    return NULL;
}

static vfs_node_t *vfs_root_finddir(vfs_node_t *node, const char *name) {
    if (!node || !(node->flags & VFS_DIR) || !name) return NULL;
    vfs_node_t *curr = node->children;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next_sibling;
    }
    return NULL;
}

static vfs_ops_t vfs_root_ops = {
    .read = NULL,
    .write = NULL,
    .open = NULL,
    .close = NULL,
    .readdir = vfs_root_readdir,
    .finddir = vfs_root_finddir,
    .create = NULL,
    .delete = NULL
};

void vfs_init(void) {
    vfs_root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!vfs_root) {
        printk("VFS: Failed to allocate root node\n");
        return;
    }
    memset(vfs_root, 0, sizeof(vfs_node_t));
    strcpy(vfs_root->name, "/");
    vfs_root->flags = VFS_DIR;
    vfs_root->ops = &vfs_root_ops;

    printk("VFS: Virtual filesystem layer initialized\n");
}

vfs_node_t *vfs_get_root(void) {
    return vfs_root;
}

vfs_node_t *vfs_lookup_path(const char *path) {
    if (!path || !vfs_root) return NULL;
    if (strcmp(path, "/") == 0) return vfs_root;

    char tmp_path[256];
    strncpy(tmp_path, path, 255);
    tmp_path[255] = '\0';

    vfs_node_t *curr = vfs_root;
    char *token = strtok(tmp_path, "/");

    while (token && curr) {
        if (!(curr->flags & VFS_DIR)) return NULL;
        vfs_node_t *found = vfs_finddir(curr, token);
        if (!found) return NULL;
        curr = found;
        token = strtok(NULL, "/");
    }

    return curr;
}

int vfs_mount(const char *path, vfs_node_t *node) {
    if (!path || !node || !vfs_root) return -1;

    if (strcmp(path, "/") == 0) {
        vfs_root = node;
        return 0;
    }

    char parent_path[256];
    char mount_name[128];
    strncpy(parent_path, path, 255);
    parent_path[255] = '\0';

    char *last_slash = strrchr(parent_path, '/');
    if (!last_slash) return -1;

    if (last_slash == parent_path) {
        strcpy(mount_name, last_slash + 1);
        strcpy(parent_path, "/");
    } else {
        strcpy(mount_name, last_slash + 1);
        *last_slash = '\0';
    }

    vfs_node_t *parent_node = vfs_lookup_path(parent_path);
    if (!parent_node || !(parent_node->flags & VFS_DIR)) return -1;

    strncpy(node->name, mount_name, 127);
    node->parent = parent_node;
    node->flags |= VFS_MOUNT;

    // Attach to parent's children
    node->next_sibling = parent_node->children;
    parent_node->children = node;

    return 0;
}

vfs_node_t *vfs_open(const char *path) {
    vfs_node_t *node = vfs_lookup_path(path);
    if (node && node->ops && node->ops->open) {
        node->ops->open(node);
    }
    return node;
}

int vfs_close(vfs_node_t *node) {
    if (node && node->ops && node->ops->close) {
        return node->ops->close(node);
    }
    return 0;
}

int vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buf) {
    if (node && node->ops && node->ops->read) {
        return node->ops->read(node, offset, size, buf);
    }
    return -1;
}

int vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buf) {
    if (node && node->ops && node->ops->write) {
        return node->ops->write(node, offset, size, buf);
    }
    return -1;
}

vfs_node_t *vfs_readdir(vfs_node_t *dir, uint32_t index) {
    if (dir && dir->ops && dir->ops->readdir) {
        return dir->ops->readdir(dir, index);
    }
    return NULL;
}

vfs_node_t *vfs_finddir(vfs_node_t *dir, const char *name) {
    if (dir && dir->ops && dir->finddir) {
        return dir->ops->finddir(dir, name);
    }
    return NULL;
}

int vfs_create(const char *path, uint32_t flags) {
    if (!path) return -1;

    char parent_path[256];
    char file_name[128];
    strncpy(parent_path, path, 255);
    parent_path[255] = '\0';

    char *last_slash = strrchr(parent_path, '/');
    if (!last_slash) return -1;

    if (last_slash == parent_path) {
        strcpy(file_name, last_slash + 1);
        strcpy(parent_path, "/");
    } else {
        strcpy(file_name, last_slash + 1);
        *last_slash = '\0';
    }

    vfs_node_t *parent_node = vfs_lookup_path(parent_path);
    if (!parent_node || !(parent_node->flags & VFS_DIR)) return -1;

    if (parent_node->ops && parent_node->ops->create) {
        return parent_node->ops->create(parent_node, file_name, flags);
    }
    return -1;
}

int vfs_delete(const char *path) {
    if (!path) return -1;

    char parent_path[256];
    char file_name[128];
    strncpy(parent_path, path, 255);
    parent_path[255] = '\0';

    char *last_slash = strrchr(parent_path, '/');
    if (!last_slash) return -1;

    if (last_slash == parent_path) {
        strcpy(file_name, last_slash + 1);
        strcpy(parent_path, "/");
    } else {
        strcpy(file_name, last_slash + 1);
        *last_slash = '\0';
    }

    vfs_node_t *parent_node = vfs_lookup_path(parent_path);
    if (!parent_node || !(parent_node->flags & VFS_DIR)) return -1;

    if (parent_node->ops && parent_node->ops->delete) {
        return parent_node->ops->delete(parent_node, file_name);
    }
    return -1;
}

int vfs_stat(const char *path, vfs_stat_t *st) {
    vfs_node_t *node = vfs_lookup_path(path);
    if (!node || !st) return -1;

    strncpy(st->name, node->name, 127);
    st->flags = node->flags;
    st->size = node->size;
    st->inode = node->inode;
    return 0;
}

void vfs_list(const char *path) {
    vfs_node_t *dir = vfs_lookup_path(path);
    if (!dir) {
        printk("ls: %s: No such file or directory\n", path);
        return;
    }

    if (!(dir->flags & VFS_DIR)) {
        printk("%-20s %u bytes\n", dir->name, dir->size);
        return;
    }

    printk("NAME                 TYPE    SIZE\n");
    printk("----                 ----    ----\n");

    uint32_t i = 0;
    vfs_node_t *child = NULL;
    while ((child = vfs_readdir(dir, i)) != NULL) {
        const char *type_str = (child->flags & VFS_DIR) ? "DIR " : "FILE";
        printk("%-20s %s    %u bytes\n", child->name, type_str, child->size);
        i++;
    }

    if (i == 0) {
        printk("(empty)\n");
    }
}
