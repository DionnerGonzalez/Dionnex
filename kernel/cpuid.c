/* ============================================================================
 * Dionnex Monolithic Kernel - CPUID Detection Subsystem Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/cpuid.h>
#include <kernel/printk.h>
#include <string.h>

static cpu_info_t g_cpu_info;

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
    return ((eax ^ ecx) & 0x00200000U) != 0;
}

static inline void native_cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(leaf), "c"(0));
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
    native_cpuid(0x80000000U, &eax, &ebx, &ecx, &edx);

    if (eax < 0x80000004U) {
        strcpy(brand, "x86 Processor");
        return;
    }

    uint32_t *b_ptr = (uint32_t*)brand;
    for (uint32_t i = 0; i < 3; i++) {
        native_cpuid(0x80000002U + i, &b_ptr[i * 4 + 0], &b_ptr[i * 4 + 1], &b_ptr[i * 4 + 2], &b_ptr[i * 4 + 3]);
    }
    brand[48] = '\0';

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

void cpuid_init(void) {
    memset(&g_cpu_info, 0, sizeof(g_cpu_info));
    cpuid_get_vendor(g_cpu_info.vendor);
    cpuid_get_brand(g_cpu_info.brand);

    g_cpu_info.is_intel = (strcmp(g_cpu_info.vendor, "GenuineIntel") == 0) ? 1 : 0;
    g_cpu_info.is_amd   = (strcmp(g_cpu_info.vendor, "AuthenticAMD") == 0) ? 1 : 0;

    if (!cpuid_supported()) {
        printk("CPU: CPUID unsupported\n");
        return;
    }

    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    native_cpuid(1, &eax, &ebx, &ecx, &edx);
    g_cpu_info.features_ecx = ecx;
    g_cpu_info.features_edx = edx;

    uint32_t base_family = (eax >> 8) & 0xFU;
    uint32_t base_model  = (eax >> 4) & 0xFU;
    g_cpu_info.stepping  = eax & 0xFU;
    g_cpu_info.family    = base_family + ((base_family == 0xFU) ? ((eax >> 20) & 0xFFU) : 0U);
    g_cpu_info.model     = base_model + ((base_family == 0x6U || base_family == 0xFU) ? (((eax >> 16) & 0xFU) << 4) : 0U);

    g_cpu_info.has_tsc    = (edx & CPUID_FEAT_EDX_TSC)  ? 1 : 0;
    g_cpu_info.has_msr    = (edx & CPUID_FEAT_EDX_MSR)  ? 1 : 0;
    g_cpu_info.has_pae    = (edx & CPUID_FEAT_EDX_PAE)  ? 1 : 0;
    g_cpu_info.has_apic   = (edx & CPUID_FEAT_EDX_APIC) ? 1 : 0;
    g_cpu_info.has_sse    = (edx & CPUID_FEAT_EDX_SSE)  ? 1 : 0;
    g_cpu_info.has_sse2   = (edx & CPUID_FEAT_EDX_SSE2) ? 1 : 0;
    g_cpu_info.has_vmx    = (ecx & CPUID_FEAT_ECX_VMX)  ? 1 : 0;
    g_cpu_info.has_x2apic = (ecx & CPUID_FEAT_ECX_X2APIC) ? 1 : 0;

    uint32_t max_ext = 0;
    native_cpuid(0x80000000U, &max_ext, &ebx, &ecx, &edx);
    if (max_ext >= 0x80000001U) {
        native_cpuid(0x80000001U, &eax, &ebx, &ecx, &edx);
        g_cpu_info.has_nx  = (edx & (1U << 20)) ? 1 : 0;
        g_cpu_info.has_svm = (ecx & (1U << 2))  ? 1 : 0;
    }

    printk("CPU: %s (%s) Family %u Model %u Stepping %u\n",
           g_cpu_info.vendor, g_cpu_info.brand,
           g_cpu_info.family, g_cpu_info.model, g_cpu_info.stepping);
}

const cpu_info_t *cpuid_get_info(void) {
    return &g_cpu_info;
}

void cpuid_print_info(void) {
    if (!cpuid_supported()) {
        printk("CPU: CPUID instruction not supported\n");
        return;
    }

    printk("CPU: Vendor:   %s\n", g_cpu_info.vendor);
    printk("CPU: Brand:    %s\n", g_cpu_info.brand);
    printk("CPU: Revision: Family=%u Model=%u Stepping=%u\n",
           g_cpu_info.family, g_cpu_info.model, g_cpu_info.stepping);

    uint32_t ecx = g_cpu_info.features_ecx;
    uint32_t edx = g_cpu_info.features_edx;

    printk("CPU: Features:");
    if (edx & CPUID_FEAT_EDX_FPU)    printk(" FPU");
    if (edx & CPUID_FEAT_EDX_TSC)    printk(" TSC");
    if (edx & CPUID_FEAT_EDX_MSR)    printk(" MSR");
    if (edx & CPUID_FEAT_EDX_PAE)    printk(" PAE");
    if (edx & CPUID_FEAT_EDX_CX8)    printk(" CX8");
    if (edx & CPUID_FEAT_EDX_APIC)   printk(" APIC");
    if (edx & CPUID_FEAT_EDX_SEP)    printk(" SEP");
    if (edx & CPUID_FEAT_EDX_MTRR)   printk(" MTRR");
    if (edx & CPUID_FEAT_EDX_PGE)    printk(" PGE");
    if (edx & CPUID_FEAT_EDX_MCA)    printk(" MCA");
    if (edx & CPUID_FEAT_EDX_CMOV)   printk(" CMOV");
    if (edx & CPUID_FEAT_EDX_PAT)    printk(" PAT");
    if (edx & CPUID_FEAT_EDX_PSE36)  printk(" PSE36");
    if (edx & CPUID_FEAT_EDX_MMX)    printk(" MMX");
    if (edx & CPUID_FEAT_EDX_FXSR)   printk(" FXSR");
    if (edx & CPUID_FEAT_EDX_SSE)    printk(" SSE");
    if (edx & CPUID_FEAT_EDX_SSE2)   printk(" SSE2");
    if (edx & CPUID_FEAT_EDX_HTT)    printk(" HTT");
    if (ecx & CPUID_FEAT_ECX_SSE3)   printk(" SSE3");
    if (ecx & CPUID_FEAT_ECX_VMX)    printk(" VMX");
    if (ecx & CPUID_FEAT_ECX_X2APIC) printk(" x2APIC");
    if (g_cpu_info.has_nx)           printk(" NX");
    if (g_cpu_info.has_svm)          printk(" SVM");
    printk("\n");
}
