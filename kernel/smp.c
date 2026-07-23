/* ============================================================================
 * Dionnex Monolithic Kernel - Symmetric Multiprocessing (SMP) Implementation
 * ============================================================================
 */
#include <kernel/smp.h>
#include <kernel/printk.h>
#include <string.h>

static smp_cpu_info_t cpus[MAX_CPUS];
static uint32_t cpu_count = 1;

static mp_floating_pointer_t *smp_find_mp_pointer(uint32_t start, uint32_t length) {
    for (uint32_t addr = start; addr < start + length; addr += 16) {
        if (memcmp((void*)addr, "_MP_", 4) == 0) {
            uint8_t *ptr = (uint8_t*)addr;
            uint8_t sum = 0;
            for (uint8_t i = 0; i < 16; i++) sum += ptr[i];
            if (sum == 0) return (mp_floating_pointer_t*)addr;
        }
    }
    return NULL;
}

void smp_init(void) {
    memset(cpus, 0, sizeof(cpus));
    cpus[0].apic_id = 0;
    cpus[0].enabled = 1;
    cpus[0].is_bsp  = 1;
    cpu_count = 1;

    // Search EBDA or BIOS ROM for MP floating pointer
    mp_floating_pointer_t *mp = smp_find_mp_pointer(0x9FC00, 1024);
    if (!mp) mp = smp_find_mp_pointer(0xF0000, 0x10000);

    if (mp && mp->phys_addr != 0) {
        mp_config_header_t *cfg = (mp_config_header_t*)mp->phys_addr;
        if (memcmp(cfg->signature, "PCMP", 4) == 0) {
            uint8_t *entry_ptr = (uint8_t*)mp->phys_addr + sizeof(mp_config_header_t);
            uint32_t parsed_cpus = 0;

            for (uint16_t i = 0; i < cfg->entry_count; i++) {
                uint8_t entry_type = *entry_ptr;
                if (entry_type == 0) { // CPU entry
                    uint8_t apic_id = entry_ptr[1];
                    uint8_t cpu_flags = entry_ptr[3];
                    if (cpu_flags & 0x01) { // Enabled
                        if (parsed_cpus < MAX_CPUS) {
                            cpus[parsed_cpus].apic_id = apic_id;
                            cpus[parsed_cpus].enabled = 1;
                            cpus[parsed_cpus].is_bsp  = (cpu_flags & 0x02) ? 1 : 0;
                            parsed_cpus++;
                        }
                    }
                    entry_ptr += 20;
                } else if (entry_type == 1) { entry_ptr += 8; }
                else if (entry_type == 2) { entry_ptr += 8; }
                else if (entry_type == 3) { entry_ptr += 8; }
                else if (entry_type == 4) { entry_ptr += 8; }
                else { break; }
            }
            if (parsed_cpus > 0) cpu_count = parsed_cpus;
        }
    }

    printk("SMP: Detected %u CPU cores (BSP APIC ID: %u)\n", cpu_count, cpus[0].apic_id);
}

uint32_t smp_get_cpu_count(void) {
    return cpu_count;
}

uint32_t smp_get_current_cpu(void) {
    return 0; // BSP
}

int smp_is_bsp(void) {
    return 1;
}

void smp_list_cpus(void) {
    printk("CPU ID  APIC ID  ROLE   STATUS\n");
    printk("------  -------  ----   ------\n");
    for (uint32_t i = 0; i < cpu_count; i++) {
        printk("%-7u %-8u %-6s %s\n",
               i, cpus[i].apic_id,
               cpus[i].is_bsp ? "BSP" : "AP",
               cpus[i].enabled ? "Online" : "Offline");
    }
}
