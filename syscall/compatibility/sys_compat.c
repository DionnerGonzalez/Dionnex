/*
 * 32-Bit System Call Compatibility Layer
 */

#include <ABI/kernel/internal/kabi.h>

s64 sys_compat_time(u32 *tloc)
{
	if (tloc)
		*tloc = 1780000000;
	return 1780000000;
}
