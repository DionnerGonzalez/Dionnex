/* ============================================================================
 * Dionnex Monolithic Kernel - PCI Express (PCIe) & MCFG ECAM Driver
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/pcie.h>
#include <drivers/pci.h>
#include <drivers/acpi.h>
#include <kernel/printk.h>
#include <mm/vmm.h>

static uint64_t ecam_phys_base = 0;
static uint8_t  ecam_start_bus = 0;
static uint8_t  ecam_end_bus   = 0;
static int      ecam_available = 0;
static uint32_t pcie_dev_count = 0;

void pcie_init(void) {
    ecam_available = 0;
    pcie_dev_count = 0;

    acpi_mcfg_table_t *mcfg = (acpi_mcfg_table_t*)acpi_find_table("MCFG");
    if (mcfg && mcfg->header.length >= sizeof(acpi_mcfg_table_t) + sizeof(acpi_mcfg_alloc_t)) {
        acpi_mcfg_alloc_t *alloc = (acpi_mcfg_alloc_t*)((uintptr_t)mcfg + sizeof(acpi_mcfg_table_t));
        if (alloc->base_address > 0 && alloc->base_address < 0x100000000ULL) {
            ecam_phys_base = alloc->base_address;
            ecam_start_bus = alloc->start_bus_number;
            ecam_end_bus   = alloc->end_bus_number;
            ecam_available = 1;
        }
    }

    pci_device_t *curr = pci_get_list();
    while (curr) {
        if (pci_find_capability(curr, PCI_CAP_ID_PCIE) != 0) {
            pcie_dev_count++;
        }
        curr = curr->next;
    }

    if (ecam_available) {
        printk("PCIe: ACPI MCFG ECAM at 0x%x (buses %u..%u), %u PCIe endpoints\n",
               (uint32_t)ecam_phys_base, ecam_start_bus, ecam_end_bus, pcie_dev_count);
    } else {
        printk("PCIe: %u PCIe-capable devices discovered (legacy CF8/CFC config)\n",
               pcie_dev_count);
    }
}

int pcie_has_ecam(void) {
    return ecam_available;
}

uint32_t pcie_get_device_count(void) {
    return pcie_dev_count;
}

uint32_t pcie_ecam_read32(uint8_t bus, uint8_t slot, uint8_t func, uint16_t offset) {
    if (!ecam_available || bus < ecam_start_bus || bus > ecam_end_bus || offset >= 4096U) {
        return pci_config_read32(bus, slot, func, (uint8_t)(offset & 0xFCU));
    }
    uint32_t phys = (uint32_t)ecam_phys_base +
                    (((uint32_t)(bus - ecam_start_bus) << 20) |
                     ((uint32_t)slot << 15) |
                     ((uint32_t)func << 12) |
                     (offset & 0xFFCU));
    volatile uint32_t *ptr = (volatile uint32_t*)vmm_ioremap(phys, 4U);
    if (!ptr) return 0xFFFFFFFFU;
    return *ptr;
}

void pcie_list(void) {
    printk("BDF       VENDOR:DEV  CAP_PTR  LINK_SPEED  LINK_WIDTH\n");
    printk("--------  ----------  -------  ----------  ----------\n");
    pci_device_t *curr = pci_get_list();
    uint32_t found = 0;
    while (curr) {
        uint8_t cap = pci_find_capability(curr, PCI_CAP_ID_PCIE);
        if (cap != 0) {
            uint16_t lnksta = pci_config_read16(curr->bus, curr->slot, curr->func, (uint8_t)(cap + 0x12U));
            uint8_t speed = lnksta & 0x0FU;
            uint8_t width = (lnksta >> 4) & 0x3FU;
            printk("%02x:%02x.%u   0x%04x:%04x 0x%02x     Gen%u        x%u\n",
                   curr->bus, curr->slot, curr->func,
                   curr->vendor_id, curr->device_id,
                   cap, speed ? speed : 1, width ? width : 1);
            found++;
        }
        curr = curr->next;
    }
    if (found == 0) {
        printk("(no PCIe capability devices found)\n");
    }
}
