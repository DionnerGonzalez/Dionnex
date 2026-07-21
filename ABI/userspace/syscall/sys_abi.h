/*
 * DionOS System Call ABI Definitions
 */

#ifndef _DIONOS_SYS_ABI_H
#define _DIONOS_SYS_ABI_H

#include <ABI/kernel/internal/kabi.h>

#define SYS_READ      0
#define SYS_WRITE     1
#define SYS_OPEN      2
#define SYS_CLOSE     3
#define SYS_STAT      4
#define SYS_FSTAT     5
#define SYS_SEEK      8
#define SYS_MMAP      9
#define SYS_MUNMAP   11
#define SYS_BRK      12
#define SYS_FORK     57
#define SYS_EXECVE   59
#define SYS_EXIT     60
#define SYS_WAIT4    61
#define SYS_KILL     62

#define SYSCALL_MAX 512

typedef s64 (*syscall_fn_t)(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);

struct syscall_entry {
	syscall_fn_t fn;
	const char *name;
	u8 num_args;
};

#endif /* _DIONOS_SYS_ABI_H */
