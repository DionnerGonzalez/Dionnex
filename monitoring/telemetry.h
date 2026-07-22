/* ============================================================================
 * Dionnex Kernel - System Monitoring & Telemetry (monitoring/telemetry.h)
 * ============================================================================
 */
#ifndef _DIONNEX_MONITORING_TELEMETRY_H
#define _DIONNEX_MONITORING_TELEMETRY_H

#include <kernel/kernel.h>

struct telemetry_metrics {
    uint32_t active_processes;
    uint32_t memory_used_bytes;
    uint32_t memory_free_bytes;
    uint32_t interrupts_handled;
    uint32_t uptime_ticks;
};

void telemetry_init(void);
void telemetry_get_metrics(struct telemetry_metrics* metrics);

#endif /* _DIONNEX_MONITORING_TELEMETRY_H */
