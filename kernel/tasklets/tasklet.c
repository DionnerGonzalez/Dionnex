/*
 * Interrupt Bottom-Half Tasklet Dispatcher
 */

#include <ABI/kernel/internal/kabi.h>

struct tasklet_struct {
	void (*func)(unsigned long data);
	unsigned long data;
	bool state;
};

void tasklet_schedule(struct tasklet_struct *t)
{
	if (!t || t->state)
		return;
	t->state = true;
	t->func(t->data);
	t->state = false;
}
