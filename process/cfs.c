/* ============================================================================
 * Dionnex Kernel - Completely Fair Scheduler (CFS) (process/cfs.c)
 * ============================================================================
 */
#include <kernel/kernel.h>
#include <kernel/kabi.h>

extern struct task_struct* get_current_task(void);
extern void set_current_task(struct task_struct* task);

static struct task_struct* runqueue_head = NULL;

void scheduler_init(void) {
    runqueue_head = NULL;
}

void cfs_add_task(struct task_struct* task) {
    if (!task) return;

    task->state = TASK_READY;

    if (!runqueue_head) {
        runqueue_head = task;
        task->next = NULL;
        return;
    }

    // Insert sorted by vruntime (virtual runtime)
    if (task->vruntime < runqueue_head->vruntime) {
        task->next = runqueue_head;
        runqueue_head = task;
        return;
    }

    struct task_struct* curr = runqueue_head;
    while (curr->next && curr->next->vruntime <= task->vruntime) {
        curr = curr->next;
    }

    task->next = curr->next;
    curr->next = task;
}

void schedule(void) {
    struct task_struct* current = get_current_task();
    if (!runqueue_head) return; // No other tasks to run

    if (current && current->state == TASK_RUNNING) {
        current->vruntime += 10; // Account for CPU time slice
        cfs_add_task(current);
    }

    // Pick leftmost node with smallest vruntime
    struct task_struct* next_task = runqueue_head;
    runqueue_head = runqueue_head->next;

    next_task->state = TASK_RUNNING;
    set_current_task(next_task);
}
