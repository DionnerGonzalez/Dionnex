/* ============================================================================
 * Dionnex Monolithic Kernel - Architecture Abstraction Layer (IA-32 / x86_64)
 * ============================================================================
 */
#ifndef _DIONNEX_ARCH_H_
#define _DIONNEX_ARCH_H_

#include <types.h>

#if defined(__x86_64__)
#define DIONNEX_ARCH_NAME "x86_64"
#define DIONNEX_WORD_BITS 64
typedef uint64_t phys_addr_t;
typedef uint64_t virt_addr_t;
typedef uint64_t reg_t;
#else
#define DIONNEX_ARCH_NAME "i686"
#define DIONNEX_WORD_BITS 32
typedef uint32_t phys_addr_t;
typedef uint32_t virt_addr_t;
typedef uint32_t reg_t;
#endif

#define DIONNEX_USER_VADDR_MIN   0x00400000U
#define DIONNEX_USER_VADDR_MAX   0xC0000000U

#endif /* _DIONNEX_ARCH_H_ */
