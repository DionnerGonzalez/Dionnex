/*
 * CPU Fault & Exception Trap Handlers
 */

#include <ABI/kernel/internal/kabi.h>
#include <kernel/core/kernel.h>

void do_page_fault(u64 fault_addr, u64 error_code)
{
	pr_err("Page fault at virtual address 0x%lx (error_code=0x%lx)\n", fault_addr, error_code);
	panic("Unhandled page fault in kernel space!");
}
