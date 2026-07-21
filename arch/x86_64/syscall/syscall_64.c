/*
 * x86_64 SYSCALL Fast System Call Dispatch Routine
 */

#include <ABI/kernel/internal/kabi.h>
#include <ABI/userspace/syscall/sys_abi.h>

extern syscall_fn_t syscall_table[];

asmlinkage s64 x86_64_sys_dispatch(u64 nr, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 a6)
{
	if (unlikely(nr >= SYSCALL_MAX || !syscall_table[nr]))
		return -1;

	return syscall_table[nr](a1, a2, a3, a4, a5, a6);
}
