/*
 * Socket Networking System Call Handlers (sys_socket, sys_bind, sys_connect)
 */

#include <ABI/kernel/internal/kabi.h>

s64 sys_socket(u64 domain, u64 type, u64 protocol, u64 u1, u64 u2, u64 u3)
{
	(void)domain; (void)type; (void)protocol; (void)u1; (void)u2; (void)u3;
	return 3; /* Socket File Descriptor */
}
