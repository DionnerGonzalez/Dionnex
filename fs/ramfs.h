/* ============================================================================
 * Dionnex Monolithic Kernel - RAM File System (RAMFS) Header
 * ============================================================================
 */
#ifndef _DIONNEX_RAMFS_H_
#define _DIONNEX_RAMFS_H_

#include <types.h>
#include <fs/vfs.h>

#define RAMFS_MAX_FILES 64
#define RAMFS_MAX_NAME  64
#define RAMFS_MAX_DATA  1024

struct ramfs_file {
    char name[RAMFS_MAX_NAME];
    uint8_t data[RAMFS_MAX_DATA];
    uint32_t size;
    uint8_t used;
    vfs_node_t vfs_node;
};

typedef struct ramfs_file ramfs_file_t;

void ramfs_init(void);
vfs_node_t *ramfs_get_root_node(void);

int ramfs_create(const char *name);
int ramfs_delete(const char *name);
int ramfs_read(const char *name, uint8_t *buf, uint32_t len);
int ramfs_write(const char *name, const uint8_t *buf, uint32_t len);
void ramfs_list(void);
int ramfs_find(const char *name);
uint32_t ramfs_get_file_count(void);

#endif /* _DIONNEX_RAMFS_H_ */
