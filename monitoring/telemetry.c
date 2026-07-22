/* ============================================================================
 * Dionnex Kernel - Telemetry Collector Implementation (monitoring/telemetry.c)
 * ============================================================================
 */
#include <monitoring/telemetry.h>

void telemetry_init(void) {
    vga_puts("[TELEMETRY] Motor de Telemetría y Monitoreo del Kernel iniciado.\n");
}

void telemetry_get_metrics(struct telemetry_metrics* metrics) {
    if (!metrics) return;

    uint32_t used = pmm_get_used_pages();
    uint32_t total = pmm_get_total_pages();

    metrics->active_processes = 2;
    metrics->memory_used_bytes = used * 4096;
    metrics->memory_free_bytes = (total - used) * 4096;
    metrics->interrupts_handled = (uint32_t)pit_get_ticks();
    metrics->uptime_ticks = (uint32_t)pit_get_ticks();
}
