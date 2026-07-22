/* ============================================================================
 * Dionnex Kernel - Kernel Application Binary Interface (ABI/kernel/kabi.h)
 * ============================================================================
 */
#ifndef _DIONNEX_ABI_KERNEL_KABI_H
#define _DIONNEX_ABI_KERNEL_KABI_H

#include <kernel/kernel.h>

#define KERNEL_ABI_VERSION_MAJOR 1
#define KERNEL_ABI_VERSION_MINOR 0
#define KERNEL_ABI_VERSION_PATCH 0

/* System Call Numbers Definition */
#define DIONNEX_SYS_READ       1
#define DIONNEX_SYS_WRITE      2
#define DIONNEX_SYS_OPEN       3
#define DIONNEX_SYS_CLOSE      4
#define DIONNEX_SYS_STAT       5
#define DIONNEX_SYS_SEEK       6
#define DIONNEX_SYS_MMAP       9
#define DIONNEX_SYS_MUNMAP     11
#define DIONNEX_SYS_FORK       57
#define DIONNEX_SYS_EXECVE     59
#define DIONNEX_SYS_EXIT       60
#define DIONNEX_SYS_GETPID     39
#define DIONNEX_SYS_TICKS      100

/* Register Frame for Interrupt Trap */
struct pt_regs {
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eax;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t orig_eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
};

#endif /* _DIONNEX_ABI_KERNEL_KABI_H */
