/* ============================================================================
 * Dionnex Monolithic Kernel - System Call Interface Header
 * ============================================================================
 */
#ifndef _DIONNEX_SYSCALL_H_
#define _DIONNEX_SYSCALL_H_

#include <types.h>
#include <kernel/idt.h>

#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_EXIT    3
#define SYS_GETPID  4
#define SYS_YIELD   5
#define SYS_UPTIME  6
#define SYS_MEMINFO 7
#define SYS_OPEN    8
#define SYS_CLOSE   9
#define SYS_FREAD   10
#define SYS_FWRITE  11
#define SYS_FORK    12
#define SYS_EXEC    13
#define SYS_SLEEP   14
#define SYS_REBOOT  15

void syscall_init(void);
void syscall_handler(registers_t *regs);

#endif /* _DIONNEX_SYSCALL_H_ */
