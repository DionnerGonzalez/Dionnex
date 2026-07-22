/* ============================================================================
 * Dionnex Kernel - Security & Capabilities Engine (security/security.h)
 * ============================================================================
 */
#ifndef _DIONNEX_SECURITY_H
#define _DIONNEX_SECURITY_H

#include <kernel/kernel.h>

#define CAP_CHOWN            (1ULL << 0)
#define CAP_DAC_OVERRIDE     (1ULL << 1)
#define CAP_NET_BIND_SERVICE (1ULL << 10)
#define CAP_SYS_ADMIN        (1ULL << 21)
#define CAP_SYS_RAWIO        (1ULL << 17)

typedef uint64_t kernel_cap_t;

struct task_security {
    uint32_t uid;
    uint32_t gid;
    uint32_t euid;
    uint32_t egid;
    kernel_cap_t cap_effective;
    kernel_cap_t cap_permitted;
};

void security_init(void);
int security_capable(const struct task_security* sec, kernel_cap_t cap);
void security_init_task(struct task_security* sec, uint32_t uid, uint32_t gid);

#endif /* _DIONNEX_SECURITY_H */
