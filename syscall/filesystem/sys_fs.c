/*
 * Filesystem VFS System Call Handlers (sys_read, sys_write, sys_open)
 */

#include <ABI/kernel/internal/kabi.h>

s64 sys_read(u64 fd, u64 buf, u64 count, u64 unused1, u64 unused2, u64 unused3)
{
	(void)fd; (void)buf; (void)count; (void)unused1; (void)unused2; (void)unused3;
	return (s64)count;
}

s64 sys_write(u64 fd, u64 buf, u64 count, u64 unused1, u64 unused2, u64 unused3)
{
	(void)fd; (void)buf; (void)count; (void)unused1; (void)unused2; (void)unused3;
	return (s64)count;
}
