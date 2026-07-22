/* ============================================================================
 * Dionnex Kernel - Performance Counter Monitor (performance/perf.h)
 * ============================================================================
 */
#ifndef _DIONNEX_PERFORMANCE_PERF_H
#define _DIONNEX_PERFORMANCE_PERF_H

#include <kernel/kernel.h>

struct perf_event {
    uint64_t cpu_cycles;
    uint64_t instructions;
    uint64_t cache_misses;
    uint64_t context_switches;
};

void perf_subsystem_init(void);
void perf_read_counters(struct perf_event* ev);

#endif /* _DIONNEX_PERFORMANCE_PERF_H */
