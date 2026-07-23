/* ============================================================================
 * Dionnex Monolithic Kernel - Process Scheduler Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_SCHEDULER_PROCESS_H_
#define _DIONNEX_SCHEDULER_PROCESS_H_

#include <types.h>
#include <kernel/process.h>

void sched_proc_init(void);
void sched_proc_add(process_t *proc);
void sched_proc_remove(process_t *proc);
void schedule_process(void);
void sched_proc_tick(void);

#endif /* _DIONNEX_SCHEDULER_PROCESS_H_ */
