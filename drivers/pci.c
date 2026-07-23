/* ============================================================================
 * Dionnex Monolithic Kernel - PCI Bus Enumeration Driver Implementation
 * ============================================================================
 */
#include <drivers/pci.h>
#include <mm/heap.h>
#include <kernel/printk.h>
#include <io.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static pci_device_t *pci_device_list = NULL;
static uint32_t pci_device_count = 0;

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (func << 8) | (offset & 0xFC) |
                                  ((uint32_t)0x80000000));
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t val = pci_config_read32(bus, slot, func, offset);
    return (uint16_t)((val >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pci_config_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t val = pci_config_read32(bus, slot, func, offset);
    return (uint8_t)((val >> ((offset & 3) * 8)) & 0xFF);
}

void pci_config_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (func << 8) | (offset & 0xFC) |
                                  ((uint32_t)0x80000000));
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}

void pci_config_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val) {
    uint32_t old_val = pci_config_read32(bus, slot, func, offset);
    uint32_t shift = (offset & 2) * 8;
    uint32_t mask = ~(0xFFFF << shift);
    uint32_t new_val = (old_val & mask) | ((uint32_t)val << shift);
    pci_config_write32(bus, slot, func, offset, new_val);
}

void pci_config_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t val) {
    uint32_t old_val = pci_config_read32(bus, slot, func, offset);
    uint32_t shift = (offset & 3) * 8;
    uint32_t mask = ~(0xFF << shift);
    uint32_t new_val = (old_val & mask) | ((uint32_t)val << shift);
    pci_config_write32(bus, slot, func, offset, new_val);
}

void pci_scan(void) {
    pci_device_count = 0;
    pci_device_list = NULL;
    pci_device_t *last_dev = NULL;

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor = pci_config_read16((uint8_t)bus, slot, func, 0x00);
                if (vendor == 0xFFFF) continue; // No device

                uint16_t device = pci_config_read16((uint8_t)bus, slot, func, 0x02);
                uint8_t class_code = pci_config_read8((uint8_t)bus, slot, func, 0x0B);
                uint8_t subclass   = pci_config_read8((uint8_t)bus, slot, func, 0x0A);
                uint8_t irq_line   = pci_config_read8((uint8_t)bus, slot, func, 0x3C);

                pci_device_t *dev = (pci_device_t*)kmalloc(sizeof(pci_device_t));
                if (!dev) continue;

                dev->bus = (uint8_t)bus;
                dev->slot = slot;
                dev->func = func;
                dev->vendor_id = vendor;
                dev->device_id = device;
                dev->class_code = class_code;
                dev->subclass = subclass;
                dev->irq_line = irq_line;
                dev->next = NULL;

                for (int b = 0; b < 6; b++) {
                    dev->bar[b] = pci_config_read32((uint8_t)bus, slot, func, 0x10 + (b * 4));
                }

                if (last_dev) {
                    last_dev->next = dev;
                } else {
                    pci_device_list = dev;
                }
                last_dev = dev;
                pci_device_count++;

                // If not multi-function device and func == 0, break function loop
                uint8_t header_type = pci_config_read8((uint8_t)bus, slot, 0, 0x0E);
                if (func == 0 && !(header_type & 0x80)) {
                    break;
                }
            }
        }
    }
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

void pci_list(void) {
    printk("BUS  SLOT  FUNC  VENDOR  DEVICE  CLASS  SUBCLASS  IRQ\n");
    printk("---  ----  ----  ------  ------  -----  --------  ---\n");

    pci_device_t *curr = pci_device_list;
    while (curr) {
        printk("%02u   %02u    %02u    0x%04x  0x%04x  0x%02x   0x%02x      %u\n",
               curr->bus, curr->slot, curr->func,
               curr->vendor_id, curr->device_id,
               curr->class_code, curr->subclass,
               curr->irq_line);
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
