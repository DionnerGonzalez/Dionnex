/* ============================================================================
 * Dionnex Monolithic Kernel - Symmetric Multiprocessing (SMP) Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/smp.h>
#include <kernel/printk.h>
#include <drivers/acpi.h>
#include <drivers/apic.h>
#include <drivers/hpet.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <string.h>

extern uint8_t ap_trampoline_start[];
extern uint8_t ap_trampoline_end[];

static smp_cpu_info_t cpus[MAX_CPUS];
static uint32_t cpu_count = 1;
static volatile uint32_t ap_booted_flag = 0;

void smp_ap_entry(void) {
    /* Enable Local APIC on AP core */
    apic_write(LAPIC_TPR, 0);
    apic_write(LAPIC_SVR, 0x1FFU);

    uint32_t my_id = apic_read(LAPIC_ID) >> 24;
    for (uint32_t i = 0; i < cpu_count; i++) {
        if (cpus[i].apic_id == (uint8_t)my_id) {
            cpus[i].enabled = 1;
        }
    }
    ap_booted_flag = 1;
    asm volatile ("cli; hlt");
    while (1) { asm volatile ("hlt"); }
}

static mp_floating_pointer_t *smp_find_mp_pointer(uint32_t start, uint32_t length) {
    for (uint32_t addr = start; addr < start + length; addr += 16) {
        if (memcmp((void*)(uintptr_t)addr, "_MP_", 4) == 0) {
            uint8_t *ptr = (uint8_t*)(uintptr_t)addr;
            uint8_t sum = 0;
            for (uint8_t i = 0; i < 16; i++) sum += ptr[i];
            if (sum == 0) return (mp_floating_pointer_t*)(uintptr_t)addr;
        }
    }
    return NULL;
}

static uint32_t smp_parse_madt(void) {
    acpi_madt_header_t *madt = (acpi_madt_header_t*)acpi_find_table("APIC");
    if (!madt || madt->header.length <= sizeof(acpi_madt_header_t)) {
        return 0;
    }

    uint32_t bsp_apic_id = apic_is_enabled() ? apic_get_id() : 0;
    uint32_t parsed = 0;
    uint32_t offset = sizeof(acpi_madt_header_t);
    const uint8_t *base = (const uint8_t*)madt;

    while (offset + 2 <= madt->header.length) {
        uint8_t type = base[offset];
        uint8_t rec_len = base[offset + 1];
        if (rec_len < 2 || offset + rec_len > madt->header.length) break;

        if (type == 0 && rec_len >= 8) {
            uint8_t apic_id = base[offset + 3];
            uint32_t flags = *(const uint32_t*)(base + offset + 4);
            if ((flags & 0x01U) && parsed < MAX_CPUS) {
                cpus[parsed].apic_id = apic_id;
                cpus[parsed].is_bsp  = (apic_id == bsp_apic_id) ? 1 : 0;
                cpus[parsed].enabled = cpus[parsed].is_bsp ? 1 : 0;
                parsed++;
            }
        }
        offset += rec_len;
    }
    return parsed;
}

static void smp_boot_aps(void) {
    if (!apic_is_enabled() || cpu_count <= 1) return;

    uint32_t tramp_size = (uint32_t)(ap_trampoline_end - ap_trampoline_start);
    if (tramp_size > 0xE0U) return;

    /* Copy AP trampoline to physical 0x8000 (low identity-mapped memory) */
    memcpy((void*)0x8000U, ap_trampoline_start, tramp_size);

    volatile uint32_t *patch_cr3   = (volatile uint32_t*)0x80F0U;
    volatile uint32_t *patch_esp   = (volatile uint32_t*)0x80F4U;
    volatile uint32_t *patch_entry = (volatile uint32_t*)0x80F8U;

    *patch_cr3   = (uint32_t)(uintptr_t)vmm_get_kernel_directory();
    *patch_entry = (uint32_t)(uintptr_t)smp_ap_entry;

    uint32_t online = 1;
    for (uint32_t i = 0; i < cpu_count; i++) {
        if (cpus[i].is_bsp) continue;

        uint32_t stack_phys = pmm_alloc_low_page();
        if (!stack_phys) continue;
        *patch_esp = stack_phys + PAGE_SIZE - 16U;

        ap_booted_flag = 0;
        apic_send_init_ipi(cpus[i].apic_id);
        hpet_delay_us(10000U); /* 10 ms INIT wait */
        apic_send_sipi_ipi(cpus[i].apic_id, 0x08U);
        hpet_delay_us(1000U);
        if (!ap_booted_flag) {
            apic_send_sipi_ipi(cpus[i].apic_id, 0x08U);
        }

        for (uint32_t wait = 0; wait < 50000U; wait++) {
            if (ap_booted_flag) break;
            hpet_delay_us(20U);
        }

        if (ap_booted_flag) {
            cpus[i].enabled = 1;
            online++;
        }
    }

    printk("SMP: %u/%u CPU cores brought online via INIT-SIPI-SIPI\n", online, cpu_count);
}

void smp_init(void) {
    memset(cpus, 0, sizeof(cpus));
    cpus[0].apic_id = (uint8_t)(apic_is_enabled() ? apic_get_id() : 0);
    cpus[0].enabled = 1;
    cpus[0].is_bsp  = 1;
    cpu_count = 1;

    uint32_t madt_cpus = smp_parse_madt();
    if (madt_cpus > 0) {
        cpu_count = madt_cpus;
        printk("SMP: MADT detected %u CPU cores (BSP APIC ID: %u)\n",
               cpu_count, cpus[0].apic_id);
        smp_boot_aps();
        return;
    }

    mp_floating_pointer_t *mp = smp_find_mp_pointer(0x9FC00U, 1024U);
    if (!mp) mp = smp_find_mp_pointer(0xF0000U, 0x10000U);

    if (mp && mp->phys_addr != 0) {
        mp_config_header_t *cfg = (mp_config_header_t*)vmm_ioremap(mp->phys_addr, PAGE_SIZE);
        if (cfg && memcmp(cfg->signature, "PCMP", 4) == 0 && cfg->length <= PAGE_SIZE) {
            uint8_t *entry_ptr = (uint8_t*)cfg + sizeof(mp_config_header_t);
            uint32_t parsed_cpus = 0;

            for (uint16_t i = 0; i < cfg->entry_count; i++) {
                uint8_t entry_type = *entry_ptr;
                if (entry_type == 0) {
                    uint8_t apic_id = entry_ptr[1];
                    uint8_t cpu_flags = entry_ptr[3];
                    if ((cpu_flags & 0x01U) && parsed_cpus < MAX_CPUS) {
                        cpus[parsed_cpus].apic_id = apic_id;
                        cpus[parsed_cpus].is_bsp  = (cpu_flags & 0x02U) ? 1 : 0;
                        cpus[parsed_cpus].enabled = cpus[parsed_cpus].is_bsp ? 1 : 0;
                        parsed_cpus++;
                    }
                    entry_ptr += 20;
                } else if (entry_type <= 4) {
                    entry_ptr += 8;
                } else {
                    break;
                }
            }
            if (parsed_cpus > 0) cpu_count = parsed_cpus;
        }
    }

    printk("SMP: Detected %u CPU cores (BSP APIC ID: %u)\n", cpu_count, cpus[0].apic_id);
    smp_boot_aps();
}

uint32_t smp_get_cpu_count(void) {
    return cpu_count;
}

uint32_t smp_get_current_cpu(void) {
    if (apic_is_enabled()) {
        uint32_t id = apic_get_id();
        for (uint32_t i = 0; i < cpu_count; i++) {
            if (cpus[i].apic_id == id) return i;
        }
    }
    return 0;
}

int smp_is_bsp(void) {
    uint32_t idx = smp_get_current_cpu();
    return cpus[idx].is_bsp ? 1 : 0;
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
