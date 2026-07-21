/*
 * Inter-Process Communication System Calls (POSIX Shared Memory & Message Queues)
 */

#include <ABI/kernel/internal/kabi.h>

s64 sys_shmget(u64 key, u64 size, u64 shmflg, u64 u1, u64 u2, u64 u3)
{
	(void)key; (void)size; (void)shmflg; (void)u1; (void)u2; (void)u3;
	return 1; /* Shared Memory Segment ID */
}
