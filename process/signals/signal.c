/*
 * POSIX Signal Delivery Engine
 */

#include <ABI/kernel/internal/kabi.h>

#define SIGHUP   1
#define SIGINT   2
#define SIGQUIT  3
#define SIGKILL  9
#define SIGSEGV 11
#define SIGTERM 15

int sys_send_signal(pid_t pid, int sig)
{
	if (pid <= 0 || sig <= 0 || sig > 32)
		return -1;

	/* Deliver pending signal to target task pending_queue */
	return 0;
}
