/* ============================================================================
 * Dionnex Monolithic Kernel - NVMe PCIe Storage Controller Driver Header
 * Adapted from Linux (drivers/nvme/host/pci.c, GPL-2.0) & FreeBSD (sys/dev/nvme/, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_NVME_H_
#define _DIONNEX_NVME_H_

#include <types.h>

void nvme_init(void);
int nvme_is_present(void);
uint64_t nvme_get_ns_sectors(void);
uint32_t nvme_get_sector_size(void);
int nvme_read_sectors(uint64_t lba, uint16_t count, uint8_t *buf);
void nvme_print_info(void);

#endif /* _DIONNEX_NVME_H_ */
