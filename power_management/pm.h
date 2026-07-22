/* ============================================================================
 * Dionnex Kernel - Power Management Core (power_management/pm.h)
 * ============================================================================
 */
#ifndef _DIONNEX_POWER_MANAGEMENT_PM_H
#define _DIONNEX_POWER_MANAGEMENT_PM_H

#include <kernel/kernel.h>

enum power_state {
    POWER_RUNNING = 0,
    POWER_SUSPEND = 1,
    POWER_HIBERNATE = 2,
    POWER_SHUTDOWN = 3
};

void power_management_init(void);
void power_set_state(enum power_state state);
void power_reboot(void);
void power_shutdown(void);

#endif /* _DIONNEX_POWER_MANAGEMENT_PM_H */
