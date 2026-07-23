/* ============================================================================
 * Dionnex Monolithic Kernel - Task Structure & Management Header
 * ============================================================================
 */
#ifndef _DIONNEX_TASK_H_
#define _DIONNEX_TASK_H_

#include <types.h>

#define TASK_RUNNING 0
#define TASK_READY   1
#define TASK_BLOCKED 2
#define TASK_DEAD    3

#define TASK_STACK_SIZE 8192

struct task {
    uint32_t pid;
    char name[32];
    uint32_t state;
    uint32_t esp;
    uint32_t *stack_base;
    uint32_t vruntime;
    uint32_t priority;
    struct task *next;
};

typedef struct task task_t;

extern task_t *current_task;

void task_init(void);
task_t *task_create(const char *name, void (*entry)(void), uint32_t priority);
void task_exit(void);
task_t *task_get_current(void);
void task_yield(void);

#endif /* _DIONNEX_TASK_H_ */
