/* ============================================================================
 * Dionnex Monolithic Kernel - PCI Bus Enumeration Driver Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/pci.h>
#include <mm/heap.h>
#include <kernel/printk.h>
#include <dionnex_rust.h>
#include <io.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static pci_device_t *pci_device_list = NULL;
static uint32_t pci_device_count = 0;

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)(((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                                  ((uint32_t)func << 8) | (offset & 0xFCU) |
                                  0x80000000U);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t val = pci_config_read32(bus, slot, func, offset);
    return (uint16_t)((val >> ((offset & 2U) * 8U)) & 0xFFFFU);
}

uint8_t pci_config_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t val = pci_config_read32(bus, slot, func, offset);
    return (uint8_t)((val >> ((offset & 3U) * 8U)) & 0xFFU);
}

void pci_config_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t address = (uint32_t)(((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                                  ((uint32_t)func << 8) | (offset & 0xFCU) |
                                  0x80000000U);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}

void pci_config_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val) {
    uint32_t old_val = pci_config_read32(bus, slot, func, offset);
    uint32_t shift = (offset & 2U) * 8U;
    uint32_t mask = ~(0xFFFFU << shift);
    uint32_t new_val = (old_val & mask) | ((uint32_t)val << shift);
    pci_config_write32(bus, slot, func, offset, new_val);
}

void pci_config_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t val) {
    uint32_t old_val = pci_config_read32(bus, slot, func, offset);
    uint32_t shift = (offset & 3U) * 8U;
    uint32_t mask = ~(0xFFU << shift);
    uint32_t new_val = (old_val & mask) | ((uint32_t)val << shift);
    pci_config_write32(bus, slot, func, offset, new_val);
}

void pci_scan(void) {
    pci_device_t *old = pci_device_list;
    while (old) {
        pci_device_t *next = old->next;
        kfree(old);
        old = next;
    }

    pci_device_count = 0;
    pci_device_list = NULL;
    pci_device_t *last_dev = NULL;

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor0 = pci_config_read16((uint8_t)bus, slot, 0, 0x00);
            if (vendor0 == 0xFFFF) continue;

            uint8_t header0 = pci_config_read8((uint8_t)bus, slot, 0, 0x0E);
            uint8_t max_func = (header0 & 0x80U) ? 8 : 1;

            for (uint8_t func = 0; func < max_func; func++) {
                uint16_t vendor = (func == 0) ? vendor0 : pci_config_read16((uint8_t)bus, slot, func, 0x00);
                if (vendor == 0xFFFF) continue;

                uint16_t device     = pci_config_read16((uint8_t)bus, slot, func, 0x02);
                uint8_t revision_id = pci_config_read8((uint8_t)bus, slot, func, 0x08);
                uint8_t prog_if     = pci_config_read8((uint8_t)bus, slot, func, 0x09);
                uint8_t subclass    = pci_config_read8((uint8_t)bus, slot, func, 0x0A);
                uint8_t class_code  = pci_config_read8((uint8_t)bus, slot, func, 0x0B);
                uint8_t header_type = pci_config_read8((uint8_t)bus, slot, func, 0x0E);
                uint8_t irq_line    = pci_config_read8((uint8_t)bus, slot, func, 0x3C);
                uint16_t status     = pci_config_read16((uint8_t)bus, slot, func, PCI_STATUS_OFFSET);

                pci_device_t *dev = (pci_device_t*)kcalloc(1, sizeof(pci_device_t));
                if (!dev) continue;

                dev->bus = (uint8_t)bus;
                dev->slot = slot;
                dev->func = func;
                dev->vendor_id = vendor;
                dev->device_id = device;
                dev->class_code = class_code;
                dev->subclass = subclass;
                dev->prog_if = prog_if;
                dev->revision_id = revision_id;
                dev->header_type = header_type & 0x7FU;
                dev->cap_ptr = (status & PCI_STATUS_CAP_LIST) ?
                               (pci_config_read8((uint8_t)bus, slot, func, 0x34) & 0xFCU) : 0;
                dev->irq_line = irq_line;
                dev->next = NULL;

                uint8_t bar_count = (dev->header_type == 0x00) ? 6 : 2;
                for (int b = 0; b < bar_count; b++) {
                    dev->bar[b] = pci_config_read32((uint8_t)bus, slot, func, (uint8_t)(0x10 + (b * 4)));
                }

                if (last_dev) {
                    last_dev->next = dev;
                } else {
                    pci_device_list = dev;
                }
                last_dev = dev;
                pci_device_count++;
            }
        }
    }
}

pci_device_t *pci_get_list(void) {
    return pci_device_list;
}

pci_device_t *pci_get_device(uint16_t vendor, uint16_t device) {
    pci_device_t *curr = pci_device_list;
    while (curr) {
        if (curr->vendor_id == vendor && curr->device_id == device) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

pci_device_t *pci_get_class(uint8_t class_code, uint8_t subclass) {
    pci_device_t *curr = pci_device_list;
    while (curr) {
        if (curr->class_code == class_code && curr->subclass == subclass) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

pci_device_t *pci_get_class_progif(uint8_t class_code, uint8_t subclass, uint8_t prog_if) {
    pci_device_t *curr = pci_device_list;
    while (curr) {
        if (curr->class_code == class_code &&
            curr->subclass == subclass &&
            curr->prog_if == prog_if) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

uint8_t pci_find_capability(pci_device_t *dev, uint8_t cap_id) {
    if (!dev || dev->cap_ptr == 0) return 0;
    uint8_t ptr = dev->cap_ptr;
    for (int guard = 0; guard < 48 && ptr >= 0x40; guard++) {
        uint8_t id = pci_config_read8(dev->bus, dev->slot, dev->func, ptr);
        if (id == cap_id) return ptr;
        ptr = pci_config_read8(dev->bus, dev->slot, dev->func, (uint8_t)(ptr + 1U)) & 0xFCU;
    }
    return 0;
}

void pci_enable_bus_master(pci_device_t *dev) {
    if (!dev) return;
    uint16_t cmd = pci_config_read16(dev->bus, dev->slot, dev->func, PCI_COMMAND_OFFSET);
    cmd |= (PCI_CMD_BUS_MASTER | PCI_CMD_MEM_SPACE);
    pci_config_write16(dev->bus, dev->slot, dev->func, PCI_COMMAND_OFFSET, cmd);
}

void pci_enable_mmio(pci_device_t *dev) {
    if (!dev) return;
    uint16_t cmd = pci_config_read16(dev->bus, dev->slot, dev->func, PCI_COMMAND_OFFSET);
    cmd |= PCI_CMD_MEM_SPACE;
    pci_config_write16(dev->bus, dev->slot, dev->func, PCI_COMMAND_OFFSET, cmd);
}

void pci_list(void) {
    printk("BUS  SLOT  FUNC  VENDOR  DEVICE  CLS/SUB  IRQ  DESCRIPTION\n");
    printk("---  ----  ----  ------  ------  -------  ---  -----------\n");

    pci_device_t *curr = pci_device_list;
    while (curr) {
        const char *desc = dionnex_rust_pci_class_name((uint8_t)curr->class_code,
                                                       (uint8_t)curr->subclass,
                                                       curr->prog_if);
        printk("%02u   %02u    %02u    0x%04x  0x%04x  %02x/%02x    %-3u  %s\n",
               curr->bus, curr->slot, curr->func,
               curr->vendor_id, curr->device_id,
               curr->class_code, curr->subclass,
               curr->irq_line, desc);
        curr = curr->next;
    }
}

uint32_t pci_get_count(void) {
    return pci_device_count;
}

void pci_init(void) {
    pci_scan();
    printk("PCI: Found %u devices\n", pci_device_count);
}
