/* ============================================================================
 * Dionnex Monolithic Kernel - CPUID Detection Subsystem Implementation
 * ============================================================================
 */
#include <kernel/cpuid.h>
#include <kernel/printk.h>
#include <string.h>

int cpuid_supported(void) {
    uint32_t eax, ecx;
    asm volatile (
        "pushfl\n"
        "popl %0\n"
        "movl %0, %1\n"
        "xorl $0x00200000, %0\n"
        "pushl %0\n"
        "popfl\n"
        "pushfl\n"
        "popl %0\n"
        "pushl %1\n"
        "popfl\n"
        : "=&r"(eax), "=&r"(ecx)
    );
    return ((eax ^ ecx) & 0x00200000) != 0;
}

static inline void native_cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(leaf));
}

void cpuid_get_vendor(char *vendor) {
    if (!cpuid_supported() || !vendor) {
        if (vendor) strcpy(vendor, "Unknown");
        return;
    }
    uint32_t eax, ebx, ecx, edx;
    native_cpuid(0, &eax, &ebx, &ecx, &edx);

    ((uint32_t*)vendor)[0] = ebx;
    ((uint32_t*)vendor)[1] = edx;
    ((uint32_t*)vendor)[2] = ecx;
    vendor[12] = '\0';
}

void cpuid_get_brand(char *brand) {
    if (!cpuid_supported() || !brand) {
        if (brand) strcpy(brand, "Generic x86 Processor");
        return;
    }

    uint32_t eax, ebx, ecx, edx;
    native_cpuid(0x80000000, &eax, &ebx, &ecx, &edx);

    if (eax < 0x80000004) {
        strcpy(brand, "x86 Processor");
        return;
    }

    uint32_t *b_ptr = (uint32_t*)brand;
    for (uint32_t i = 0; i < 3; i++) {
        native_cpuid(0x80000002 + i, &b_ptr[i*4 + 0], &b_ptr[i*4 + 1], &b_ptr[i*4 + 2], &b_ptr[i*4 + 3]);
    }
    brand[48] = '\0';

    // Trim leading spaces if present
    char *p = brand;
    while (*p == ' ') p++;
    if (p != brand) {
        memmove(brand, p, strlen(p) + 1);
    }
}

void cpuid_get_features(uint32_t *ecx, uint32_t *edx) {
    if (!cpuid_supported()) {
        if (ecx) *ecx = 0;
        if (edx) *edx = 0;
        return;
    }
    uint32_t eax, ebx;
    native_cpuid(1, &eax, &ebx, ecx, edx);
}

int cpuid_has_feature(uint32_t feature_bit) {
    uint32_t ecx, edx;
    cpuid_get_features(&ecx, &edx);
    return (edx & feature_bit) != 0;
}

void cpuid_print_info(void) {
    if (!cpuid_supported()) {
        printk("CPU: CPUID instruction not supported\n");
        return;
    }

    char vendor[13];
    char brand[49];
    cpuid_get_vendor(vendor);
    cpuid_get_brand(brand);

    printk("CPU: Vendor: %s\n", vendor);
    printk("CPU: Brand: %s\n", brand);

    uint32_t ecx, edx;
    cpuid_get_features(&ecx, &edx);

    printk("CPU: Features:");
    if (edx & CPUID_FEAT_EDX_FPU)   printk(" FPU");
    if (edx & CPUID_FEAT_EDX_TSC)   printk(" TSC");
    if (edx & CPUID_FEAT_EDX_MSR)   printk(" MSR");
    if (edx & CPUID_FEAT_EDX_PAE)   printk(" PAE");
    if (edx & CPUID_FEAT_EDX_CX8)   printk(" CX8");
    if (edx & CPUID_FEAT_EDX_APIC)  printk(" APIC");
    if (edx & CPUID_FEAT_EDX_SEP)   printk(" SEP");
    if (edx & CPUID_FEAT_EDX_MTRR)  printk(" MTRR");
    if (edx & CPUID_FEAT_EDX_PGE)   printk(" PGE");
    if (edx & CPUID_FEAT_EDX_MCA)   printk(" MCA");
    if (edx & CPUID_FEAT_EDX_CMOV)  printk(" CMOV");
    if (edx & CPUID_FEAT_EDX_PAT)   printk(" PAT");
    if (edx & CPUID_FEAT_EDX_PSE36) printk(" PSE36");
    if (edx & CPUID_FEAT_EDX_MMX)   printk(" MMX");
    if (edx & CPUID_FEAT_EDX_FXSR)  printk(" FXSR");
    if (edx & CPUID_FEAT_EDX_SSE)   printk(" SSE");
    if (edx & CPUID_FEAT_EDX_SSE2)  printk(" SSE2");
    if (edx & CPUID_FEAT_EDX_HTT)   printk(" HTT");
    printk("\n");
}
