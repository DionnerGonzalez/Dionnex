/*
 * DionOS Kernel Panic Handler & Registers Dump
 */

#include <kernel/core/kernel.h>
#include <stdarg.h>

void panic(const char *fmt, ...)
{
	(void)fmt;

	pr_err("========================================================\n");
	pr_err("KERNEL PANIC: Fatal exception occurred in kernel mode!\n");
	pr_err("System halted. Please reboot hardware or check kernel log.\n");
	pr_err("========================================================\n");

	/* Disable interrupts and enter infinite halt loop */
	__asm__ __volatile__("cli");
	for (;;) {
		__asm__ __volatile__("hlt");
	}
}
EXPORT_SYMBOL(panic);
