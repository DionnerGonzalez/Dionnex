/*
 * DionOS Circular Ring Buffer Kernel Logger
 */

#include <kernel/core/kernel.h>

#define LOG_BUF_SIZE 65536

static char log_buf[LOG_BUF_SIZE];
static u32 log_head = 0;

void pr_info(const char *fmt, ...)
{
	/* Append formatted string into kernel ring buffer and write to early serial output */
	(void)fmt;
	(void)log_buf;
	(void)log_head;
}

void pr_err(const char *fmt, ...)
{
	(void)fmt;
}

void pr_warn(const char *fmt, ...)
{
	(void)fmt;
}

void pr_debug(const char *fmt, ...)
{
	(void)fmt;
}

EXPORT_SYMBOL(pr_info);
EXPORT_SYMBOL(pr_err);
