/*
 * Kernel High-Resolution Timers and Jiffies Wheel
 */

#include <ABI/kernel/internal/kabi.h>

volatile u64 jiffies = 0;

struct timer_list {
	u64 expires;
	void (*function)(u64 data);
	u64 data;
};

void timer_tick(void)
{
	jiffies++;
	/* Check expired timer wheel queues */
}
