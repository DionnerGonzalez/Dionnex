/* ============================================================================
 * Dionnex Monolithic Kernel - AHCI (SATA) Controller Driver Header
 * Adapted from Linux (drivers/ata/ahci.c, GPL-2.0-or-later) & FreeBSD (sys/dev/ahci/, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_AHCI_H_
#define _DIONNEX_AHCI_H_

#include <types.h>

#define AHCI_MAX_PORTS 32

#define AHCI_SIG_ATA   0x00000101U
#define AHCI_SIG_ATAPI 0xEB140101U

typedef struct ahci_port_info {
    uint8_t  present;
    uint8_t  port_num;
    char     model[41];
    uint64_t sectors;
    uint32_t cmd_list_phys;
    void    *cmd_list_virt;
    uint32_t fis_phys;
    void    *fis_virt;
    uint32_t cmd_tbl_phys;
    void    *cmd_tbl_virt;
    uint32_t dma_buf_phys;
    void    *dma_buf_virt;
} ahci_port_info_t;

void ahci_init(void);
int ahci_is_present(void);
uint32_t ahci_get_port_count(void);
int ahci_read_sectors(uint8_t port_idx, uint64_t lba, uint16_t count, uint8_t *buf);
void ahci_list_ports(void);

#endif /* _DIONNEX_AHCI_H_ */
