/*
 * Kernel Threads & User Thread Context Allocation
 */

#include <ABI/kernel/internal/kabi.h>

struct thread_info {
	u64 flags;
	u32 cpu;
	s32 preempt_count;
};

void thread_exit(long exit_code)
{
	(void)exit_code;
	/* Clean thread resources and notify parent process */
}
