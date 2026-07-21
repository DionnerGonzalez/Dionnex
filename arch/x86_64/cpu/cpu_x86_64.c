/*
 * x86_64 Processor Initialization and Long Mode MSR Configurations
 */

#include <ABI/kernel/internal/kabi.h>

#define MSR_EFER 0xC0000080
#define MSR_STAR 0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_FMASK 0xC0000084

static inline void wrmsr(u32 msr, u64 val)
{
	u32 low = val & 0xFFFFFFFF;
	u32 high = val >> 32;
	__asm__ __volatile__("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

void init_x86_64_cpu(void)
{
	/* Configure SYSCALL target address in LSTAR MSR */
}
