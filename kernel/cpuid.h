/* ============================================================================
 * Dionnex Monolithic Kernel - CPUID Detection Subsystem Header
 * ============================================================================
 */
#ifndef _DIONNEX_CPUID_H_
#define _DIONNEX_CPUID_H_

#include <types.h>

#define CPUID_FEAT_EDX_FPU     (1 << 0)
#define CPUID_FEAT_EDX_TSC     (1 << 4)
#define CPUID_FEAT_EDX_MSR     (1 << 5)
#define CPUID_FEAT_EDX_PAE     (1 << 6)
#define CPUID_FEAT_EDX_CX8     (1 << 8)
#define CPUID_FEAT_EDX_APIC    (1 << 9)
#define CPUID_FEAT_EDX_SEP     (1 << 11)
#define CPUID_FEAT_EDX_MTRR    (1 << 12)
#define CPUID_FEAT_EDX_PGE     (1 << 13)
#define CPUID_FEAT_EDX_MCA     (1 << 14)
#define CPUID_FEAT_EDX_CMOV    (1 << 15)
#define CPUID_FEAT_EDX_PAT     (1 << 16)
#define CPUID_FEAT_EDX_PSE36   (1 << 17)
#define CPUID_FEAT_EDX_MMX     (1 << 23)
#define CPUID_FEAT_EDX_FXSR    (1 << 24)
#define CPUID_FEAT_EDX_SSE     (1 << 25)
#define CPUID_FEAT_EDX_SSE2    (1 << 26)
#define CPUID_FEAT_EDX_HTT     (1 << 28)

int cpuid_supported(void);
void cpuid_get_vendor(char *vendor);
void cpuid_get_brand(char *brand);
void cpuid_get_features(uint32_t *ecx, uint32_t *edx);
int cpuid_has_feature(uint32_t feature_bit);
void cpuid_print_info(void);

#endif /* _DIONNEX_CPUID_H_ */
