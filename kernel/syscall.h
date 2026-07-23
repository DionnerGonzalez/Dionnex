/* ============================================================================
 * Dionnex Monolithic Kernel - System Call Interface Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_SYSCALL_H_
#define _DIONNEX_SYSCALL_H_

#include <types.h>
#include <kernel/idt.h>

#define SYS_EXIT          1
#define SYS_FORK          2
#define SYS_READ          3
#define SYS_WRITE         4
#define SYS_OPEN          5
#define SYS_CLOSE         6
#define SYS_WAITPID       7
#define SYS_EXEC          11
#define SYS_SBRK          12
#define SYS_STAT          18
#define SYS_LSEEK         19
#define SYS_GETPID        20
#define SYS_GETPPID       21
#define SYS_KILL          22
#define SYS_MMAP          23
#define SYS_MUNMAP        24
#define SYS_IOCTL         54
#define SYS_GETTIMEOFDAY  78
#define SYS_UNAME         122
#define SYS_NANOSLEEP     162
#define SYS_INIT_MODULE   175
#define SYS_DELETE_MODULE 176
#define SYS_QUERY_MODULE  177
#define SYS_EXIT_GROUP    252

// Internal aliases for existing functions
#define SYS_YIELD         5
#define SYS_UPTIME        6
#define SYS_MEMINFO       7
#define SYS_FREAD         10
#define SYS_FWRITE        11
#define SYS_SLEEP         14
#define SYS_REBOOT        15

void syscall_init(void);
void syscall_handler(registers_t *regs);

#endif /* _DIONNEX_SYSCALL_H_ */
