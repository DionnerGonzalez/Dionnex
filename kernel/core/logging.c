/*
 * DionOS Circular Ring Buffer Kernel Logger
 */

#include <kernel/core/kernel.h>

#define LOG_BUF_SIZE 65536

static char log_buf[LOG_BUF_SIZE];
static u32 log_head = 0;

void pr_info(const char *fmt, ...)
{
	vga_puts("[INFO] ");
	vga_puts(fmt);
}

void pr_err(const char *fmt, ...)
{
	vga_puts("[ERR] ");
	vga_puts(fmt);
}

void pr_warn(const char *fmt, ...)
{
	vga_puts("[WARN] ");
	vga_puts(fmt);
}

void pr_debug(const char *fmt, ...)
{
	vga_puts("[DBG] ");
	vga_puts(fmt);
}

EXPORT_SYMBOL(pr_info);
EXPORT_SYMBOL(pr_err);

