/* ============================================================================
 * Dionnex Monolithic Kernel - Process Management Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_PROCESS_H_
#define _DIONNEX_PROCESS_H_

#include <types.h>

#define PROC_MAX         64
#define PROC_NAME_LEN    64
#define PROC_MAX_FDS     16
#define PROC_STACK_SIZE  (1024 * 1024)   // 1MB user stack
#define PROC_CODE_BASE   0x08048000     // Virtual code base
#define PROC_STACK_BASE  0xBFFFFFFF     // Top of user stack
#define PROC_HEAP_BASE   0x08100000     // Virtual heap base

enum proc_state {
    PROC_NEW = 0,
    PROC_READY,
    PROC_RUNNING,
    PROC_BLOCKED,
    PROC_ZOMBIE,
    PROC_DEAD
};

struct process_struct {
    uint32_t pid;
    char name[PROC_NAME_LEN];
    uint32_t state;
    uint32_t *page_directory;
    uint32_t esp0;
    uint32_t *kernel_stack;
    uint32_t user_esp;
    uint32_t user_eip;
    uint32_t entry_point;
    uint32_t brk;
    uint32_t brk_max;
    int fds[PROC_MAX_FDS];
    uint32_t exit_code;
    uint32_t vruntime;
    struct process_struct *parent;
    struct process_struct *next;
};

typedef struct process_struct process_t;

extern process_t *current_process;

void process_init(void);
process_t *process_create(const char *name, uint8_t *elf_data, uint32_t elf_size);
void process_exit(int code);
void process_destroy(process_t *proc);
process_t *process_get_current(void);
process_t *process_get_by_pid(uint32_t pid);
uint32_t process_get_count(void);
void process_list(void);

#endif /* _DIONNEX_PROCESS_H_ */
