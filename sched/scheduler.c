/* ============================================================================
 * Dionnex Monolithic Kernel - Vruntime Preemptive Scheduler Implementation
 * ============================================================================
 */
#include <sched/scheduler.h>
#include <sched/task.h>
#include <mm/pmm.h>
#include <kernel/printk.h>

extern task_t *current_task;
extern task_t *task_list_head;
extern void context_switch(uint32_t *old_esp, uint32_t new_esp);

void scheduler_init(void) {
    printk("Scheduler: Vruntime scheduler initialized\n");
}

void scheduler_add_task(task_t *task) {
    if (!task || !task_list_head) return;
    task_t *curr = task_list_head;
    while (curr->next) {
        if (curr == task) return; // Already present
        curr = curr->next;
    }
    curr->next = task;
    task->next = NULL;
}

void scheduler_remove_task(task_t *task) {
    if (!task || !task_list_head) return;

    if (task_list_head == task) {
        task_list_head = task_list_head->next;
        return;
    }

    task_t *curr = task_list_head;
    while (curr->next) {
        if (curr->next == task) {
            curr->next = task->next;
            return;
        }
        curr = curr->next;
    }
}

uint32_t scheduler_get_task_count(void) {
    uint32_t count = 0;
    task_t *curr = task_list_head;
    while (curr) {
        count++;
        curr = curr->next;
    }
    return count;
}

void schedule(void) {
    if (!current_task || !task_list_head) return;

    /* Clean up DEAD tasks if any */
    if (current_task->state == TASK_DEAD) {
        task_t *dead_task = current_task;
        scheduler_remove_task(dead_task);
    }

    /* Find READY task with minimum vruntime */
    task_t *min_task = NULL;
    uint32_t min_vruntime = 0xFFFFFFFF;

    task_t *curr = task_list_head;
    while (curr) {
        if (curr->state == TASK_READY || curr->state == TASK_RUNNING) {
            if (curr->vruntime < min_vruntime) {
                min_vruntime = curr->vruntime;
                min_task = curr;
            }
        }
        curr = curr->next;
    }

    if (!min_task) return;

    if (min_task != current_task) {
        task_t *prev = current_task;

        if (prev->state == TASK_RUNNING) {
            prev->state = TASK_READY;
            prev->vruntime += (10 - prev->priority);
        }

        current_task = min_task;
        current_task->state = TASK_RUNNING;

        context_switch(&prev->esp, current_task->esp);
    }
}

void scheduler_list(void) {
    printk("PID  NAME             STATE     VRUNTIME\n");
    printk("---  ----             -----     --------\n");

    task_t *curr = task_list_head;
    while (curr) {
        const char *st_str = "UNKNOWN";
        switch (curr->state) {
            case TASK_RUNNING: st_str = "RUNNING"; break;
            case TASK_READY:   st_str = "READY  "; break;
            case TASK_BLOCKED: st_str = "BLOCKED"; break;
            case TASK_DEAD:    st_str = "DEAD   "; break;
        }
        printk("%u    %s          %s   %u\n", curr->pid, curr->name, st_str, curr->vruntime);
        curr = curr->next;
    }
}
