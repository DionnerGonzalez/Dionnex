/* ============================================================================
 * Dionnex Kernel - Hardware Abstraction Layer (hardware_abstraction/hal.h)
 * ============================================================================
 */
#ifndef _DIONNEX_HARDWARE_ABSTRACTION_HAL_H
#define _DIONNEX_HARDWARE_ABSTRACTION_HAL_H

#include <kernel/kernel.h>

struct cpu_info {
    char vendor[13];
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint32_t features_edx;
    uint32_t features_ecx;
};

void hal_init(void);
void hal_get_cpu_info(struct cpu_info* info);
void hal_delay_us(uint32_t us);

#endif /* _DIONNEX_HARDWARE_ABSTRACTION_HAL_H */
