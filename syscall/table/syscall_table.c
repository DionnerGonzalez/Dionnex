/*
 * DionOS Master System Call Vector Table
 */

#include <ABI/kernel/internal/kabi.h>
#include <ABI/userspace/syscall/sys_abi.h>

extern s64 sys_read(u64 fd, u64 buf, u64 count, u64 unused1, u64 unused2, u64 unused3);
extern s64 sys_write(u64 fd, u64 buf, u64 count, u64 unused1, u64 unused2, u64 unused3);
extern s64 sys_fork(u64 unused1, u64 unused2, u64 unused3, u64 unused4, u64 unused5, u64 unused6);
extern s64 sys_exit(u64 status, u64 unused1, u64 unused2, u64 unused3, u64 unused4, u64 unused5);

syscall_fn_t syscall_table[SYSCALL_MAX] = {
	[SYS_READ]  = (syscall_fn_t)sys_read,
	[SYS_WRITE] = (syscall_fn_t)sys_write,
	[SYS_FORK]  = (syscall_fn_t)sys_fork,
	[SYS_EXIT]  = (syscall_fn_t)sys_exit,
};

void syscall_init(void)
{
	/* Register syscall vector table into hardware target architecture */
}
