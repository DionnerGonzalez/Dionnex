/*
 * Completely Fair Scheduler (CFS) Subsystem
 */

#include <ABI/kernel/internal/kabi.h>

struct sched_entity {
	u64 vruntime;
	u64 exec_start;
	u64 sum_exec_runtime;
	s32 load_weight;
};

struct cfs_rq {
	u64 min_vruntime;
	unsigned long nr_running;
	struct sched_entity *curr;
};

static struct cfs_rq global_cfs_rq;

void cfs_update_curr(struct cfs_rq *rq, u64 delta_exec)
{
	if (!rq || !rq->curr)
		return;

	rq->curr->sum_exec_runtime += delta_exec;
	rq->curr->vruntime += delta_exec * (1024 / rq->curr->load_weight);
	if (rq->curr->vruntime > rq->min_vruntime)
		rq->min_vruntime = rq->curr->vruntime;
}

void cfs_init(void)
{
	global_cfs_rq.min_vruntime = 0;
	global_cfs_rq.nr_running = 0;
	global_cfs_rq.curr = (struct sched_entity *)0;
}
