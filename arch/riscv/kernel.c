/*
 * RISC-V (RV64GC) Sv39 / Sv48 Virtual Memory and SBI Call Vector
 */

#include <ABI/kernel/internal/kabi.h>

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1

static inline u64 sbi_call(u64 extension, u64 function, u64 arg0, u64 arg1)
{
	register u64 a0 __asm__("a0") = arg0;
	register u64 a1 __asm__("a1") = arg1;
	register u64 a6 __asm__("a6") = function;
	register u64 a7 __asm__("a7") = extension;
	__asm__ __volatile__("ecall" : "+r"(a0) : "r"(a1), "r"(a6), "r"(a7) : "memory");
	return a0;
}

void riscv64_init(void)
{
	/* Initialize Sv39 page tables and delegate S-mode interrupts */
	sbi_call(0x10, 0, 0, 0);
}
