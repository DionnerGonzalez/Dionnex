/*
 * Deferred Execution Workqueue Engine
 */

#include <ABI/kernel/internal/kabi.h>

struct work_struct {
	void (*func)(struct work_struct *work);
	struct work_struct *next;
};

void queue_work(struct work_struct *work)
{
	/* Enqueue work into asynchronous worker kernel threads */
	(void)work;
}
