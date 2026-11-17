/* ============================================================================
 * Dionnex Monolithic Kernel - Process Management Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_PROCESS_H_
#define _DIONNEX_PROCESS_H_

#include <types.h>
#include <mm/vmm.h>
#include <fs/vfs.h>

#define PROC_MAX         64
#define PROC_NAME_LEN    64
#define PROC_MAX_FDS     16
#define PROC_STACK_SIZE  (64 * 1024)    /* 64KB initial user stack */
#define PROC_CODE_BASE   0x08048000U    /* Virtual code base */
#define PROC_STACK_BASE  0xBFFFF000U    /* Top of user stack (page-aligned) */
#define PROC_HEAP_BASE   0x08100000U    /* Virtual heap base */
#define PROC_MMAP_BASE   0x40000000U    /* Virtual mmap base */

#define SIGKILL 9
#define SIGSEGV 11
#define SIGTERM 15
#define SIGCHLD 17

enum proc_state {
    PROC_NEW = 0,
    PROC_READY,
    PROC_RUNNING,
    PROC_BLOCKED,
    PROC_ZOMBIE,
    PROC_DEAD
};

typedef struct fd_entry {
    uint8_t     used;
    uint8_t     is_console;
    uint32_t    flags;
    uint32_t    offset;
    vfs_node_t *node;
} fd_entry_t;

struct process_struct {
    uint32_t          pid;
    char              name[PROC_NAME_LEN];
    uint32_t          state;
    page_directory_t *page_directory;
    uint32_t          esp0;
    uint32_t         *kernel_stack;
    uint32_t          kctx_esp;
    uint32_t          in_trampoline;
    uint32_t          user_esp;
    uint32_t          user_eip;
    uint32_t          entry_point;
    uint32_t          brk;
    uint32_t          brk_max;
    uint32_t          mmap_next;
    fd_entry_t        fd_table[PROC_MAX_FDS];
    uint32_t          pending_signals;
    uint32_t          exit_code;
    uint32_t          vruntime;
    struct process_struct *parent;
    struct process_struct *list_next;
    struct process_struct *sched_next;
};

typedef struct process_struct process_t;

extern process_t *current_process;

void process_init(void);
process_t *process_create(const char *name, const uint8_t *elf_data, uint32_t elf_size);
int process_run(process_t *proc);
void process_exit(int code);
void process_destroy(process_t *proc);
int process_alloc_fd(process_t *proc, vfs_node_t *node, uint32_t flags);
fd_entry_t *process_get_fd(process_t *proc, int fd);
int process_close_fd(process_t *proc, int fd);
process_t *process_get_current(void);
process_t *process_get_by_pid(uint32_t pid);
uint32_t process_get_count(void);
void process_list(void);

#endif /* _DIONNEX_PROCESS_H_ */
