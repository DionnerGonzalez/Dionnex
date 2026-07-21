/*
 * Process Control Block (task_struct) and Process Lifecycle Management
 */

#include <ABI/kernel/internal/kabi.h>

#define TASK_RUNNING         0
#define TASK_INTERRUPTIBLE   1
#define TASK_UNINTERRUPTIBLE 2
#define TASK_STOPPED         4
#define TASK_ZOMBIE          8

struct task_struct {
	volatile long state;
	pid_t pid;
	pid_t tgid;
	char comm[16];
	u64 cr3;
	virt_addr_t kernel_stack;
	struct task_struct *parent;
	struct task_struct *next;
};

static struct task_struct init_task = {
	.state = TASK_RUNNING,
	.pid = 0,
	.tgid = 0,
	.comm = "swapper/0",
	.cr3 = 0,
	.kernel_stack = 0,
	.parent = (struct task_struct *)0,
	.next = (struct task_struct *)0
};

struct task_struct *get_current_task(void)
{
	return &init_task;
}

pid_t sys_fork_process(void)
{
	/* Copy task_struct and allocate new kernel stack */
	return 101;
}
