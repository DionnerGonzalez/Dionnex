/*
 * x86 MMU Paging and CR3 Register Management
 */

#include <ABI/kernel/internal/kabi.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_RW      (1ULL << 1)
#define PAGE_USER    (1ULL << 2)
#define PAGE_NX      (1ULL << 63)

void x86_switch_mmu_context(u64 pml4_phys_addr)
{
	__asm__ __volatile__("mov %0, %%cr3" : : "r" (pml4_phys_addr) : "memory");
}
