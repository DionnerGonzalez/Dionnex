/*
 * Memory Protection Unit (MPU) & W^X Enforcement
 */

#include <ABI/kernel/internal/kabi.h>

void enforce_wx_protection(virt_addr_t addr, size_t len)
{
	(void)addr;
	(void)len;
	/* Mark kernel pages as either Writable XOR Executable */
}
