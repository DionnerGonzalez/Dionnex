/*
 * DionOS Core Kernel Header Definitions
 */

#ifndef _DIONOS_KERNEL_CORE_H
#define _DIONOS_KERNEL_CORE_H

#include <ABI/kernel/internal/kabi.h>

void pr_info(const char *fmt, ...);
void pr_err(const char *fmt, ...);
void pr_warn(const char *fmt, ...);
void pr_debug(const char *fmt, ...);

void panic(const char *fmt, ...) __attribute__((noreturn));
void start_kernel(void);

#endif /* _DIONOS_KERNEL_CORE_H */
