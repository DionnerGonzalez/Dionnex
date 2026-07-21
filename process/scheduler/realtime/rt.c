/*
 * POSIX Real-Time Scheduler (SCHED_FIFO and SCHED_RR)
 */

#include <ABI/kernel/internal/kabi.h>

#define MAX_RT_PRIO 100

struct rt_rq {
	unsigned long rt_nr_running;
	struct rt_prio_array {
		u64 bitmap[4];
	} active;
};

void rt_schedule_next(struct rt_rq *rq)
{
	if (!rq || rq->rt_nr_running == 0)
		return;
	/* Dispatch highest priority real-time thread */
}
