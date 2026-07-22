/* ============================================================================
 * Dionnex Kernel - Userspace Application Binary Interface (ABI/userspace/uabi.h)
 * ============================================================================
 */
#ifndef _DIONNEX_ABI_USERSPACE_UABI_H
#define _DIONNEX_ABI_USERSPACE_UABI_H

#include <kernel/kernel.h>

/* File Access Modes */
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0040
#define O_TRUNC     0x0200
#define O_APPEND    0x0400

/* Process Exit Flags */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* Memory Protection Flags */
#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4
#define PROT_NONE   0x0

/* System Statistics Structure */
struct sysinfo_data {
    uint32_t uptime_sec;
    uint32_t total_ram_pages;
    uint32_t free_ram_pages;
    uint32_t active_tasks;
    uint32_t total_syscalls;
};

#endif /* _DIONNEX_ABI_USERSPACE_UABI_H */
