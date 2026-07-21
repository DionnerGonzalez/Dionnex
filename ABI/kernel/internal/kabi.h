/*
 * Dionnex Kernel Internal ABI Definition
 * Standard C11 Kernel Types and Export Definitions
 */

#ifndef _DIONNEX_KABI_H
#define _DIONNEX_KABI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

typedef u64 phys_addr_t;
typedef u64 virt_addr_t;
typedef s32 pid_t;
typedef s32 tid_t;

#define ASMLINKAGE __attribute__((regparm(0)))
#define EXPORT_SYMBOL(sym) const void *__kstrtab_##sym __attribute__((section("__ksymtab"))) = (void *)&sym
#define __init __attribute__((section(".init.text")))
#define __exit __attribute__((section(".exit.text")))
#define __aligned(x) __attribute__((aligned(x)))
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

struct kernel_version {
	u16 major;
	u16 minor;
	u16 patch;
	const char *extra;
};

#endif /* _DIONOS_KABI_H */
