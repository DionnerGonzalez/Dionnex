/*
 * Dionnex Core Kernel Header Definitions
 */

#ifndef _DIONNEX_KERNEL_CORE_H
#define _DIONNEX_KERNEL_CORE_H

#include <kernel.h>

#define asmlinkage
#define EXPORT_SYMBOL(x)
#define __init
#define u32 uint32_t

void pr_info(const char *fmt, ...);
void pr_err(const char *fmt, ...);
void pr_warn(const char *fmt, ...);
void pr_debug(const char *fmt, ...);

void panic(const char *fmt, ...) __attribute__((noreturn));
void start_kernel(void);
const char *get_kernel_banner(void);

#endif /* _DIONNEX_KERNEL_CORE_H */

