/*
 * PID Namespaces Subsystem for Container Isolation
 */

#include <ABI/kernel/internal/kabi.h>

struct pid_namespace {
	u32 level;
	pid_t max_pid;
	struct pid_namespace *parent;
};

static struct pid_namespace init_pid_ns = {
	.level = 0,
	.max_pid = 32768,
	.parent = (struct pid_namespace *)0
};

struct pid_namespace *get_init_pid_ns(void)
{
	return &init_pid_ns;
}
