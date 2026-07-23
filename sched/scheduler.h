/* ============================================================================
 * Dionnex Monolithic Kernel - Process Scheduler Header
 * ============================================================================
 */
#ifndef _DIONNEX_SCHEDULER_H_
#define _DIONNEX_SCHEDULER_H_

#include <sched/task.h>

void scheduler_init(void);
void schedule(void);
void scheduler_add_task(task_t *task);
void scheduler_remove_task(task_t *task);
uint32_t scheduler_get_task_count(void);
void scheduler_list(void);

#endif /* _DIONNEX_SCHEDULER_H_ */
