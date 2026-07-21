/*
 * Intel CPU Identification and Feature Enumeration (x86/x86_64)
 */

#include <ABI/kernel/internal/kabi.h>

struct cpuinfo_x86 {
	u8 x86_vendor;
	u8 x86_family;
	u8 x86_model;
	u8 x86_stepping;
	char vendor_id[16];
	u32 cpuid_level;
	u32 x86_capability[10];
};

static struct cpuinfo_x86 boot_cpu_data;

static inline void cpuid(u32 op, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
	__asm__ __volatile__("cpuid"
	                     : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
	                     : "0" (op));
}

void detect_intel_cpu(void)
{
	u32 eax, ebx, ecx, edx;
	cpuid(0, &eax, &ebx, &ecx, &edx);
	boot_cpu_data.cpuid_level = eax;
	*(u32 *)&boot_cpu_data.vendor_id[0] = ebx;
	*(u32 *)&boot_cpu_data.vendor_id[4] = edx;
	*(u32 *)&boot_cpu_data.vendor_id[8] = ecx;
	boot_cpu_data.vendor_id[12] = '\0';
}
