/* ============================================================================
 * Dionnex Monolithic Kernel - PCI Bus Enumeration Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_PCI_H_
#define _DIONNEX_PCI_H_

#include <types.h>

struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t class_code;
    uint16_t subclass;
    uint32_t bar[6];
    uint8_t irq_line;
    struct pci_device *next;
};

typedef struct pci_device pci_device_t;

void pci_init(void);
void pci_scan(void);
uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t pci_config_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

void pci_config_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);
void pci_config_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val);
void pci_config_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t val);

pci_device_t *pci_get_device(uint16_t vendor, uint16_t device);
pci_device_t *pci_get_class(uint8_t class_code, uint8_t subclass);
void pci_list(void);
uint32_t pci_get_count(void);

#endif /* _DIONNEX_PCI_H_ */
