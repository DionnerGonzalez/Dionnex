/* ============================================================================
 * Dionnex Kernel - Power Management ACPI Implementation (power_management/pm.c)
 * ============================================================================
 */
#include <power_management/pm.h>

static enum power_state current_power_state = POWER_RUNNING;

void power_management_init(void) {
    current_power_state = POWER_RUNNING;
    vga_puts("[PM] Gestor de Energía y ACPI inicializado correctamente.\n");
}

void power_set_state(enum power_state state) {
    current_power_state = state;
    if (state == POWER_SHUTDOWN) {
        power_shutdown();
    } else if (state == POWER_SUSPEND) {
        vga_puts("[PM] Entrando en estado de suspensión (S3 ACPI)...\n");
    }
}

void power_reboot(void) {
    vga_puts("[PM] Reiniciando sistema operativo...\n");
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE); // 8042 Keyboard Controller Reset Line
    while (1) hlt();
}

void power_shutdown(void) {
    vga_puts("[PM] Apagando el sistema operativo Dionnex...\n");
    outb(0x604, 0x2000); // QEMU ACPI shutdown port
    outb(0xB004, 0x2000); // Bochs / Older QEMU shutdown
    while (1) hlt();
}
