/* ============================================================================
 * Dionnex Kernel - Userspace Sysfs Core (userspace_interface/sys_interface.c)
 * ============================================================================
 */
#include <userspace_interface/sys_interface.h>

void sys_interface_init(void) {
    vga_puts("[SYSFS] Interfaz de Espacio de Usuario /sys inicializada.\n");
}

int sys_interface_read_entry(const char* path, char* out_buf, size_t max_len) {
    if (!path || !out_buf || max_len == 0) return -1;

    if (strcmp(path, "/sys/kernel/version") == 0) {
        strncpy(out_buf, "Dionnex Kernel v1.0.0 (x86_64)", max_len - 1);
        return 0;
    } else if (strcmp(path, "/sys/kernel/uptime") == 0) {
        uint32_t ticks = (uint32_t)pit_get_ticks();
        strncpy(out_buf, "Uptime OK", max_len - 1);
        return 0;
    }

    return -1;
}
