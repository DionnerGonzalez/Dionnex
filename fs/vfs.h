/* ============================================================================
 * Dionnex Monolithic Kernel - Virtual Filesystem (VFS) Header
 * ============================================================================
 */
#ifndef _DIONNEX_VFS_H_
#define _DIONNEX_VFS_H_

#include <types.h>

#define VFS_FILE     0x01
#define VFS_DIR      0x02
#define VFS_CHARDEV  0x04
#define VFS_BLOCKDEV 0x08
#define VFS_MOUNT    0x10

struct vfs_node;

struct vfs_ops {
    int (*read)(struct vfs_node *node, uint32_t offset, uint32_t size, uint8_t *buf);
    int (*write)(struct vfs_node *node, uint32_t offset, uint32_t size, const uint8_t *buf);
    int (*open)(struct vfs_node *node);
    int (*close)(struct vfs_node *node);
    struct vfs_node *(*readdir)(struct vfs_node *node, uint32_t index);
    struct vfs_node *(*finddir)(struct vfs_node *node, const char *name);
    int (*create)(struct vfs_node *node, const char *name, uint32_t flags);
    int (*delete)(struct vfs_node *node, const char *name);
};

typedef struct vfs_ops vfs_ops_t;

struct vfs_node {
    char name[128];
    uint32_t flags;
    uint32_t size;
    uint32_t inode;
    struct vfs_node *parent;
    struct vfs_node *children;
    struct vfs_node *next_sibling;
    void *private_data;
    vfs_ops_t *ops;
};

typedef struct vfs_node vfs_node_t;

struct vfs_stat {
    char name[128];
    uint32_t flags;
    uint32_t size;
    uint32_t inode;
};

typedef struct vfs_stat vfs_stat_t;

void vfs_init(void);
vfs_node_t *vfs_get_root(void);
int vfs_mount(const char *path, vfs_node_t *node);
vfs_node_t *vfs_open(const char *path);
int vfs_close(vfs_node_t *node);
int vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buf);
int vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buf);
vfs_node_t *vfs_readdir(vfs_node_t *dir, uint32_t index);
vfs_node_t *vfs_finddir(vfs_node_t *dir, const char *name);
int vfs_create(const char *path, uint32_t flags);
int vfs_delete(const char *path);
int vfs_stat(const char *path, vfs_stat_t *st);
void vfs_list(const char *path);

#endif /* _DIONNEX_VFS_H_ */
