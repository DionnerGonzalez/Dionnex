/* ============================================================================
 * Dionnex Monolithic Kernel - Device Filesystem (/dev) Header
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_DEVFS_H_
#define _DIONNEX_DEVFS_H_

#include <types.h>
#include <fs/vfs.h>

void devfs_init(void);
vfs_node_t *devfs_get_root_node(void);

#endif /* _DIONNEX_DEVFS_H_ */
