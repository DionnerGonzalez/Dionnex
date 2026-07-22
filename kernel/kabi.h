/* ============================================================================
 * Dionnex Kernel - Kernel Application Binary Interface (kernel/kabi.h)
 * ============================================================================
 */
#ifndef _DIONNEX_KABI_H
#define _DIONNEX_KABI_H

#include <kernel/kernel.h>

/* Dionnex System Call Numbers */
#define SYS_EXIT      1
#define SYS_FORK      2
#define SYS_READ      3
#define SYS_WRITE     4
#define SYS_OPEN      5
#define SYS_CLOSE     6
#define SYS_GETPID    20
#define SYS_MALLOC    45
#define SYS_TICKS     100
#define SYS_EXEC      11

/* Process States */
#define TASK_RUNNING     0
#define TASK_READY       1
#define TASK_SLEEPING    2
#define TASK_TERMINATED  3

/* Memory Flags */
#define PAGE_PRESENT  0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER     0x4

/* Register state passed to ISR/IRQ handlers */
struct cpu_registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

/* Process Control Block (PCB) structure */
struct task_struct {
    uint32_t pid;
    char name[32];
    uint32_t state;
    uint32_t esp;
    uint32_t eip;
    uint32_t cr3;
    uint32_t time_slice;
    uint32_t priority;
    uint32_t vruntime; // For CFS
    void* stack;
    struct task_struct* next;
};

#endif /* _DIONNEX_KABI_H */
