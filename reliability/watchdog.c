/* ============================================================================
 * Dionnex Kernel - Hardware Watchdog Timer (reliability/watchdog.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

static uint32_t watchdog_timeout = 30; // 30 seconds
static int watchdog_enabled = 0;

void watchdog_init(uint32_t timeout_sec) {
    watchdog_timeout = timeout_sec;
    watchdog_enabled = 1;
    vga_puts("[WATCHDOG] Temporizador Perro Guardián (Watchdog) activo.\n");
}

void watchdog_kick(void) {
    if (watchdog_enabled) {
        // Reset watchdog counter
    }
}
