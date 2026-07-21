/*
 * DionOS In-Kernel Debugger (KDB)
 */

#include <ABI/kernel/internal/kabi.h>
#include <kernel/core/kernel.h>

void kdb_enter(const char *reason)
{
	pr_info("KDB Debugger Active. Reason: %s\n", reason);
	/* Trap to serial console debug CLI */
}
