/* ============================================================================
 * Dionnex Kernel - NUMA Memory Node Management (memory_advanced/numa.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

struct numa_node {
    uint32_t node_id;
    uint32_t total_memory_mb;
    uint32_t free_memory_mb;
    int cpus_mask;
};

static struct numa_node numa_nodes[2];

void numa_init(void) {
    numa_nodes[0].node_id = 0;
    numa_nodes[0].total_memory_mb = 64;
    numa_nodes[0].free_memory_mb = 64;
    numa_nodes[0].cpus_mask = 0x01;

    numa_nodes[1].node_id = 1;
    numa_nodes[1].total_memory_mb = 64;
    numa_nodes[1].free_memory_mb = 64;
    numa_nodes[1].cpus_mask = 0x02;

    vga_puts("[NUMA] Gestor de Arquitectura de Memoria NUMA (2 Nodos) activo.\n");
}
