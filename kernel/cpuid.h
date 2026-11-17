/* ============================================================================
 * Dionnex Monolithic Kernel - CPUID & MSR Detection Subsystem Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_CPUID_H_
#define _DIONNEX_CPUID_H_

#include <types.h>

#define CPUID_FEAT_EDX_FPU     (1U << 0)
#define CPUID_FEAT_EDX_TSC     (1U << 4)
#define CPUID_FEAT_EDX_MSR     (1U << 5)
#define CPUID_FEAT_EDX_PAE     (1U << 6)
#define CPUID_FEAT_EDX_CX8     (1U << 8)
#define CPUID_FEAT_EDX_APIC    (1U << 9)
#define CPUID_FEAT_EDX_SEP     (1U << 11)
#define CPUID_FEAT_EDX_MTRR    (1U << 12)
#define CPUID_FEAT_EDX_PGE     (1U << 13)
#define CPUID_FEAT_EDX_MCA     (1U << 14)
#define CPUID_FEAT_EDX_CMOV    (1U << 15)
#define CPUID_FEAT_EDX_PAT     (1U << 16)
#define CPUID_FEAT_EDX_PSE36   (1U << 17)
#define CPUID_FEAT_EDX_MMX     (1U << 23)
#define CPUID_FEAT_EDX_FXSR    (1U << 24)
#define CPUID_FEAT_EDX_SSE     (1U << 25)
#define CPUID_FEAT_EDX_SSE2    (1U << 26)
#define CPUID_FEAT_EDX_HTT     (1U << 28)

#define CPUID_FEAT_ECX_SSE3    (1U << 0)
#define CPUID_FEAT_ECX_VMX     (1U << 5)
#define CPUID_FEAT_ECX_SSSE3   (1U << 9)
#define CPUID_FEAT_ECX_X2APIC  (1U << 21)

typedef struct cpu_info {
    char vendor[13];
    char brand[49];
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint32_t features_edx;
    uint32_t features_ecx;
    uint8_t is_intel;
    uint8_t is_amd;
    uint8_t has_tsc;
    uint8_t has_msr;
    uint8_t has_apic;
    uint8_t has_sse;
    uint8_t has_sse2;
    uint8_t has_pae;
    uint8_t has_nx;
    uint8_t has_vmx;
    uint8_t has_svm;
    uint8_t has_x2apic;
} cpu_info_t;

void cpuid_init(void);
const cpu_info_t *cpuid_get_info(void);
int cpuid_supported(void);
void cpuid_get_vendor(char *vendor);
void cpuid_get_brand(char *brand);
void cpuid_get_features(uint32_t *ecx, uint32_t *edx);
int cpuid_has_feature(uint32_t feature_bit);
void cpuid_print_info(void);

#endif /* _DIONNEX_CPUID_H_ */
