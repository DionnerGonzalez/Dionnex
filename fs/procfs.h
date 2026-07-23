/* ============================================================================
 * Dionnex Monolithic Kernel - Process Filesystem (PROCFS) Header
 * ============================================================================
 */
#ifndef _DIONNEX_PROCFS_H_
#define _DIONNEX_PROCFS_H_

#include <types.h>
#include <fs/vfs.h>

void procfs_init(void);
vfs_node_t *procfs_get_root_node(void);

#endif /* _DIONNEX_PROCFS_H_ */
