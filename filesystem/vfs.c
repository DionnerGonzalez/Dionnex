/* ============================================================================
 * Dionnex Kernel - Virtual File System Core (filesystem/vfs.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

#define MAX_VFS_FILES 32

struct vfs_node {
    char name[64];
    char content[1024];
    uint32_t size;
    int is_used;
};

static struct vfs_node vfs_files[MAX_VFS_FILES];

void vfs_init(void) {
    memset(vfs_files, 0, sizeof(vfs_files));
}

int vfs_add_file(const char* name, const char* content, uint32_t size) {
    for (int i = 0; i < MAX_VFS_FILES; i++) {
        if (!vfs_files[i].is_used) {
            strncpy(vfs_files[i].name, name, 63);
            if (content && size > 0) {
                uint32_t len = size < 1023 ? size : 1023;
                memcpy(vfs_files[i].content, content, len);
                vfs_files[i].content[len] = '\0';
                vfs_files[i].size = len;
            } else {
                vfs_files[i].content[0] = '\0';
                vfs_files[i].size = 0;
            }
            vfs_files[i].is_used = 1;
            return 0;
        }
    }
    return -1; // VFS full
}

void vfs_ls(void) {
    kprintf("VFS Root Directory Listing (/):\n");
    int count = 0;
    for (int i = 0; i < MAX_VFS_FILES; i++) {
        if (vfs_files[i].is_used) {
            kprintf("  - %-20s (%u bytes)\n", vfs_files[i].name, vfs_files[i].size);
            count++;
        }
    }
    if (count == 0) {
        kprintf("  (no files)\n");
    }
}

const char* vfs_read_file(const char* name) {
    for (int i = 0; i < MAX_VFS_FILES; i++) {
        if (vfs_files[i].is_used && strcmp(vfs_files[i].name, name) == 0) {
            return vfs_files[i].content;
        }
    }
    return NULL;
}
