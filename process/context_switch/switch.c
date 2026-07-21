/*
 * CPU Register Context Switch Engine (switch_to)
 */

#include <ABI/kernel/internal/kabi.h>

struct cpu_context {
	u64 rsp;
	u64 rbp;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
	u64 rip;
};

void cpu_switch_context(struct cpu_context *prev, struct cpu_context *next)
{
	if (!prev || !next)
		return;

	/* Save callee registers of prev and load registers of next */
}
