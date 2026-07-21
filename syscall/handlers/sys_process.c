/*
 * Process Control System Call Handlers (sys_fork, sys_execve, sys_exit)
 */

#include <ABI/kernel/internal/kabi.h>

s64 sys_fork(u64 unused1, u64 unused2, u64 unused3, u64 unused4, u64 unused5, u64 unused6)
{
	(void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5; (void)unused6;
	return 1024; /* Fork child PID */
}

s64 sys_exit(u64 status, u64 unused1, u64 unused2, u64 unused3, u64 unused4, u64 unused5)
{
	(void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
	/* Terminate current process and yield execution */
	return (s64)status;
}
