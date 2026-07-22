/* ============================================================================
 * Dionnex Kernel - Capability Security Implementation (security/capability.c)
 * ============================================================================
 */
#include <security/security.h>

void security_init(void) {
    vga_puts("[SECURITY] Motor de Capacidades y Seguridad POSIX inicializado.\n");
}

int security_capable(const struct task_security* sec, kernel_cap_t cap) {
    if (!sec) return 0;
    if (sec->euid == 0) return 1; // Root has full privileges
    return (sec->cap_effective & cap) != 0;
}

void security_init_task(struct task_security* sec, uint32_t uid, uint32_t gid) {
    if (!sec) return;
    sec->uid = uid;
    sec->gid = gid;
    sec->euid = uid;
    sec->egid = gid;
    if (uid == 0) {
        sec->cap_effective = ~0ULL;
        sec->cap_permitted = ~0ULL;
    } else {
        sec->cap_effective = 0;
        sec->cap_permitted = 0;
    }
}
