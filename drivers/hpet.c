/* ============================================================================
 * Dionnex Monolithic Kernel - High Precision Event Timer (HPET) Driver
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/hpet.h>
#include <drivers/acpi.h>
#include <kernel/printk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <io.h>

static volatile uint32_t *hpet_mmio = NULL;
static uint32_t hpet_period_fs = 0;
static uint32_t hpet_freq_hz = 0;
static int hpet_ready = 0;

static inline uint32_t hpet_read32(uint32_t offset) {
    if (!hpet_mmio) return 0;
    return *(volatile uint32_t*)((uintptr_t)hpet_mmio + offset);
}

static inline void hpet_write32(uint32_t offset, uint32_t val) {
    if (!hpet_mmio) return;
    *(volatile uint32_t*)((uintptr_t)hpet_mmio + offset) = val;
}

void hpet_init(void) {
    hpet_ready = 0;
    acpi_hpet_table_t *tbl = (acpi_hpet_table_t*)acpi_find_table("HPET");
    if (!tbl || tbl->address.address == 0 || tbl->address.address >= 0x100000000ULL) {
        printk("HPET: ACPI HPET table not available\n");
        return;
    }

    uint32_t phys = (uint32_t)tbl->address.address;
    hpet_mmio = (volatile uint32_t*)vmm_ioremap(phys, PAGE_SIZE);
    if (!hpet_mmio) {
        printk("HPET: Failed to ioremap 0x%x\n", phys);
        return;
    }

    uint32_t cap_lo = hpet_read32(HPET_REG_CAP_ID);
    uint32_t cap_hi = hpet_read32(HPET_REG_CAP_ID + 4U);
    hpet_period_fs = cap_hi;
    if (hpet_period_fs == 0 || hpet_period_fs > 100000000U) {
        printk("HPET: Invalid clock period (%u fs)\n", hpet_period_fs);
        return;
    }

    hpet_freq_hz = (uint32_t)(1000000000000000ULL / (uint64_t)hpet_period_fs);

    uint32_t cfg = hpet_read32(HPET_REG_GEN_CONF);
    cfg |= 0x01U; /* ENABLE_CNF */
    hpet_write32(HPET_REG_GEN_CONF, cfg);

    uint32_t timers = ((cap_lo >> 8) & 0x1FU) + 1U;
    hpet_ready = 1;
    printk("HPET: Active at phys 0x%x (%u timers, %u MHz, 64-bit=%u)\n",
           phys, timers, hpet_freq_hz / 1000000U, (cap_lo >> 13) & 1U);
}

int hpet_is_available(void) {
    return hpet_ready;
}

uint64_t hpet_read_counter(void) {
    if (!hpet_ready) return 0;
    uint32_t hi1, lo, hi2;
    do {
        hi1 = hpet_read32(HPET_REG_MAIN_CNT + 4U);
        lo  = hpet_read32(HPET_REG_MAIN_CNT);
        hi2 = hpet_read32(HPET_REG_MAIN_CNT + 4U);
    } while (hi1 != hi2);
    return ((uint64_t)hi1 << 32) | lo;
}

uint32_t hpet_get_freq_hz(void) {
    return hpet_freq_hz;
}

void hpet_delay_us(uint32_t microseconds) {
    if (microseconds == 0) return;
    if (!hpet_ready || hpet_freq_hz == 0) {
        for (volatile uint32_t i = 0; i < microseconds * 20U; i++) {
            cpu_pause();
        }
        return;
    }
    uint64_t start = hpet_read_counter();
    uint64_t ticks = ((uint64_t)microseconds * (uint64_t)hpet_freq_hz) / 1000000ULL;
    if (ticks == 0) ticks = 1;
    while ((hpet_read_counter() - start) < ticks) {
        cpu_pause();
    }
}
