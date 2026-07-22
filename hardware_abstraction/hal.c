/* ============================================================================
 * Dionnex Kernel - Hardware Abstraction Layer Implementation (hardware_abstraction/hal.c)
 * ============================================================================
 */
#include <hardware_abstraction/hal.h>

void hal_init(void) {
    vga_puts("[HAL] Hardware Abstraction Layer (HAL) x86/x86_64 cargado.\n");
}

void hal_get_cpu_info(struct cpu_info* info) {
    if (!info) return;

    uint32_t ebx, ecx, edx;

    // CPUID EAX=0 (Get Vendor ID)
    __asm__ __volatile__ (
        "cpuid"
        : "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );

    ((uint32_t*)info->vendor)[0] = ebx;
    ((uint32_t*)info->vendor)[1] = edx;
    ((uint32_t*)info->vendor)[2] = ecx;
    info->vendor[12] = '\0';
}

void hal_delay_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 1000; i++) {
        __asm__ __volatile__("nop");
    }
}
