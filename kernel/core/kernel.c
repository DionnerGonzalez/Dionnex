/*
 * DionOS Core Kernel Execution Entry & Symbol Table Manager
 */

#include <kernel/core/kernel.h>

static const char *kernel_banner = "Dionnex Modular Monolithic Kernel v1.0.0-RELEASE (c) 2026";

const char *get_kernel_banner(void)
{
	return kernel_banner;
}

EXPORT_SYMBOL(get_kernel_banner);
