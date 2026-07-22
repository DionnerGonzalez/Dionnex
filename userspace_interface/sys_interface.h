/* ============================================================================
 * Dionnex Kernel - Userspace Sysfs Interface (userspace_interface/sys_interface.h)
 * ============================================================================
 */
#ifndef _DIONNEX_USERSPACE_INTERFACE_SYS_INTERFACE_H
#define _DIONNEX_USERSPACE_INTERFACE_SYS_INTERFACE_H

#include <kernel/kernel.h>

void sys_interface_init(void);
int sys_interface_read_entry(const char* path, char* out_buf, size_t max_len);

#endif /* _DIONNEX_USERSPACE_INTERFACE_SYS_INTERFACE_H */
