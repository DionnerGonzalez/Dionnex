/* ============================================================================
 * Dionnex Kernel - Virtual File System VFS Layer (fs/vfs.c)
 * ============================================================================
 */
#include <kernel.h>

struct vfs_node {
    char name[64];
    uint32_t flags; // 1 = File, 2 = Directory
    uint32_t size;
    uint32_t inode;
    const char* content;
};

static struct vfs_node vfs_root[16];
static uint32_t vfs_count = 0;

void vfs_init(void) {
    vfs_count = 0;
    vga_puts("[VFS] Virtual File System VFS montado en nodo raiz '/'.\n");
}

int vfs_add_file(const char* name, const char* content, uint32_t size) {
    if (vfs_count >= 16) return -1;
    
    size_t i = 0;
    while (name[i] && i < 63) {
        vfs_root[vfs_count].name[i] = name[i];
        i++;
    }
    vfs_root[vfs_count].name[i] = '\0';

    vfs_root[vfs_count].flags = 1;
    vfs_root[vfs_count].content = content;
    vfs_root[vfs_count].size = size;
    vfs_root[vfs_count].inode = vfs_count + 1;
    vfs_count++;
    return 0;
}

void vfs_ls(void) {
    for (uint32_t i = 0; i < vfs_count; i++) {
        vga_puts("  - ");
        vga_puts(vfs_root[i].name);
        vga_puts(" [size=");
        vga_putc('0' + (vfs_root[i].size % 10));
        vga_puts(" bytes]\n");
    }
}

const char* vfs_read_file(const char* name) {
    for (uint32_t i = 0; i < vfs_count; i++) {
        // Simple match
        if (vfs_root[i].name[0] == name[0] && vfs_root[i].name[1] == name[1]) {
            return vfs_root[i].content;
        }
    }
    return NULL;
}
