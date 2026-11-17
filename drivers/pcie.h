/* ============================================================================
 * Dionnex Monolithic Kernel - PCI Express (PCIe) & MCFG ECAM Header
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_PCIE_H_
#define _DIONNEX_PCIE_H_

#include <types.h>

#define PCI_CAP_ID_PCIE 0x10U

void pcie_init(void);
void pcie_list(void);
uint32_t pcie_get_device_count(void);
int pcie_has_ecam(void);
uint32_t pcie_ecam_read32(uint8_t bus, uint8_t slot, uint8_t func, uint16_t offset);

#endif /* _DIONNEX_PCIE_H_ */
