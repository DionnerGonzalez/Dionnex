/* ============================================================================
 * Dionnex Monolithic Kernel - AHCI (SATA) Controller Driver
 * Adapted from Linux (drivers/ata/ahci.c, GPL-2.0-or-later) & FreeBSD (sys/dev/ahci/, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/ahci.h>
#include <drivers/pci.h>
#include <kernel/printk.h>
#include <mm/vmm.h>
#include <mm/dma.h>
#include <string.h>
#include <io.h>

#define HBA_CAP       0x00U
#define HBA_GHC       0x04U
#define HBA_IS        0x08U
#define HBA_PI        0x0CU
#define HBA_VS        0x10U

#define HBA_GHC_AE    (1U << 31)
#define HBA_GHC_HR    (1U << 0)

#define PORT_BASE(p)  (0x100U + ((uint32_t)(p) * 0x80U))
#define PORT_CLB      0x00U
#define PORT_CLBU     0x04U
#define PORT_FB       0x08U
#define PORT_FBU      0x0CU
#define PORT_IS       0x10U
#define PORT_IE       0x14U
#define PORT_CMD      0x18U
#define PORT_TFD      0x20U
#define PORT_SIG      0x24U
#define PORT_SSTS     0x28U
#define PORT_SERR     0x30U
#define PORT_CI       0x38U

#define PORT_CMD_ST   (1U << 0)
#define PORT_CMD_FRE  (1U << 4)
#define PORT_CMD_FR   (1U << 14)
#define PORT_CMD_CR   (1U << 15)

#define FIS_TYPE_REG_H2D 0x27U
#define ATA_CMD_IDENTIFY 0xECU
#define ATA_CMD_READ_DMA_EXT 0x25U

typedef struct ahci_cmd_header {
    uint8_t  cfl : 5;
    uint8_t  a   : 1;
    uint8_t  w   : 1;
    uint8_t  p   : 1;
    uint8_t  r   : 1;
    uint8_t  b   : 1;
    uint8_t  c   : 1;
    uint8_t  rsv0 : 1;
    uint8_t  pmp : 4;
    uint16_t prdtl;
    uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsv1[4];
} __attribute__((packed)) ahci_cmd_header_t;

typedef struct ahci_prdt_entry {
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsv0;
    uint32_t dbc;
} __attribute__((packed)) ahci_prdt_entry_t;

typedef struct ahci_cmd_table {
    uint8_t cfis[64];
    uint8_t acmd[16];
    uint8_t rsv[48];
    ahci_prdt_entry_t prdt[1];
} __attribute__((packed)) ahci_cmd_table_t;

typedef struct fis_reg_h2d {
    uint8_t  fis_type;
    uint8_t  pmport : 4;
    uint8_t  rsv0   : 3;
    uint8_t  c      : 1;
    uint8_t  command;
    uint8_t  featurel;
    uint8_t  lba0;
    uint8_t  lba1;
    uint8_t  lba2;
    uint8_t  device;
    uint8_t  lba3;
    uint8_t  lba4;
    uint8_t  lba5;
    uint8_t  featureh;
    uint8_t  countl;
    uint8_t  counth;
    uint8_t  icc;
    uint8_t  control;
    uint8_t  rsv1[4];
} __attribute__((packed)) fis_reg_h2d_t;

static volatile uint8_t *abar = NULL;
static ahci_port_info_t ahci_ports[AHCI_MAX_PORTS];
static uint32_t active_ports = 0;
static int ahci_present = 0;

static inline uint32_t ahci_read32(uint32_t off) {
    return *(volatile uint32_t*)(abar + off);
}

static inline void ahci_write32(uint32_t off, uint32_t val) {
    *(volatile uint32_t*)(abar + off) = val;
}

static void ahci_port_stop(uint8_t p) {
    uint32_t base = PORT_BASE(p);
    uint32_t cmd = ahci_read32(base + PORT_CMD);
    cmd &= ~(PORT_CMD_ST | PORT_CMD_FRE);
    ahci_write32(base + PORT_CMD, cmd);

    for (uint32_t i = 0; i < 500000U; i++) {
        uint32_t cur = ahci_read32(base + PORT_CMD);
        if (!(cur & (PORT_CMD_FR | PORT_CMD_CR))) break;
        cpu_pause();
    }
}

static void ahci_port_start(uint8_t p) {
    uint32_t base = PORT_BASE(p);
    for (uint32_t i = 0; i < 500000U; i++) {
        if (!(ahci_read32(base + PORT_CMD) & PORT_CMD_CR)) break;
        cpu_pause();
    }
    uint32_t cmd = ahci_read32(base + PORT_CMD);
    cmd |= PORT_CMD_FRE | PORT_CMD_ST;
    ahci_write32(base + PORT_CMD, cmd);
}

static int ahci_exec_cmd(ahci_port_info_t *info, uint8_t ata_cmd, uint64_t lba, uint16_t count, uint32_t bytes) {
    uint32_t base = PORT_BASE(info->port_num);

    for (uint32_t spin = 0; spin < 500000U; spin++) {
        uint32_t tfd = ahci_read32(base + PORT_TFD);
        if (!(tfd & 0x88U)) break;
        cpu_pause();
    }

    ahci_write32(base + PORT_IS, 0xFFFFFFFFU);

    ahci_cmd_header_t *hdr = (ahci_cmd_header_t*)info->cmd_list_virt;
    memset(hdr, 0, sizeof(ahci_cmd_header_t));
    hdr->cfl = sizeof(fis_reg_h2d_t) / 4U;
    hdr->w = 0;
    hdr->prdtl = 1;
    hdr->ctba = info->cmd_tbl_phys;
    hdr->ctbau = 0;

    ahci_cmd_table_t *tbl = (ahci_cmd_table_t*)info->cmd_tbl_virt;
    memset(tbl, 0, sizeof(ahci_cmd_table_t));
    tbl->prdt[0].dba = info->dma_buf_phys;
    tbl->prdt[0].dbau = 0;
    tbl->prdt[0].dbc = (bytes - 1U) | (1U << 31);

    fis_reg_h2d_t *fis = (fis_reg_h2d_t*)tbl->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;
    fis->command = ata_cmd;
    fis->device = (ata_cmd == ATA_CMD_IDENTIFY) ? 0x00U : 0x40U;
    fis->lba0 = (uint8_t)(lba & 0xFFU);
    fis->lba1 = (uint8_t)((lba >> 8) & 0xFFU);
    fis->lba2 = (uint8_t)((lba >> 16) & 0xFFU);
    fis->lba3 = (uint8_t)((lba >> 24) & 0xFFU);
    fis->lba4 = (uint8_t)((lba >> 32) & 0xFFU);
    fis->lba5 = (uint8_t)((lba >> 40) & 0xFFU);
    fis->countl = (uint8_t)(count & 0xFFU);
    fis->counth = (uint8_t)((count >> 8) & 0xFFU);

    ahci_write32(base + PORT_CI, 1U);

    for (uint32_t i = 0; i < 2000000U; i++) {
        uint32_t ci = ahci_read32(base + PORT_CI);
        uint32_t is = ahci_read32(base + PORT_IS);
        if ((is & (1U << 30)) != 0) return -1;
        if ((ci & 1U) == 0) return 0;
        cpu_pause();
    }
    return -1;
}

static void ahci_fix_ata_str(char *dst, const uint16_t *words, uint32_t word_count) {
    for (uint32_t i = 0; i < word_count; i++) {
        dst[i * 2U]     = (char)((words[i] >> 8) & 0xFFU);
        dst[i * 2U + 1] = (char)(words[i] & 0xFFU);
    }
    dst[word_count * 2U] = '\0';
    for (int i = (int)(word_count * 2U) - 1; i >= 0; i--) {
        if (dst[i] == ' ' || dst[i] == '\0') dst[i] = '\0';
        else break;
    }
}

static void ahci_probe_port(uint8_t p) {
    uint32_t base = PORT_BASE(p);
    uint32_t ssts = ahci_read32(base + PORT_SSTS);
    uint8_t det = ssts & 0x0FU;
    uint8_t ipm = (ssts >> 8) & 0x0FU;
    if (det != 3U || ipm != 1U) return;

    uint32_t sig = ahci_read32(base + PORT_SIG);
    if (sig != AHCI_SIG_ATA) return;

    ahci_port_info_t *info = &ahci_ports[active_ports];
    memset(info, 0, sizeof(ahci_port_info_t));
    info->port_num = p;

    dma_buffer_t b_cl, b_fis, b_tbl, b_io;
    if (dma_alloc_coherent(4096U, &b_cl) != 0 ||
        dma_alloc_coherent(4096U, &b_fis) != 0 ||
        dma_alloc_coherent(4096U, &b_tbl) != 0 ||
        dma_alloc_coherent(4096U, &b_io) != 0) {
        return;
    }
    info->cmd_list_virt = b_cl.virt;  info->cmd_list_phys = b_cl.phys;
    info->fis_virt      = b_fis.virt; info->fis_phys      = b_fis.phys;
    info->cmd_tbl_virt  = b_tbl.virt; info->cmd_tbl_phys  = b_tbl.phys;
    info->dma_buf_virt  = b_io.virt;  info->dma_buf_phys  = b_io.phys;

    ahci_port_stop(p);
    ahci_write32(base + PORT_CLB, info->cmd_list_phys);
    ahci_write32(base + PORT_CLBU, 0);
    ahci_write32(base + PORT_FB, info->fis_phys);
    ahci_write32(base + PORT_FBU, 0);
    ahci_write32(base + PORT_SERR, 0xFFFFFFFFU);
    ahci_write32(base + PORT_IS, 0xFFFFFFFFU);
    ahci_port_start(p);

    if (ahci_exec_cmd(info, ATA_CMD_IDENTIFY, 0, 1, 512U) == 0) {
        const uint16_t *id = (const uint16_t*)info->dma_buf_virt;
        ahci_fix_ata_str(info->model, &id[27], 20U);
        uint64_t lba48 = ((uint64_t)id[103] << 48) | ((uint64_t)id[102] << 32) |
                         ((uint64_t)id[101] << 16) | (uint64_t)id[100];
        uint32_t lba28 = ((uint32_t)id[61] << 16) | (uint32_t)id[60];
        info->sectors = lba48 ? lba48 : (uint64_t)lba28;
        info->present = 1;
        active_ports++;
        printk("AHCI: Port %u SATA drive '%s' (%u MB)\n",
               p, info->model, (uint32_t)(info->sectors / 2048ULL));
    }
}

void ahci_init(void) {
    ahci_present = 0;
    active_ports = 0;
    memset(ahci_ports, 0, sizeof(ahci_ports));

    pci_device_t *dev = pci_get_class_progif(0x01, 0x06, 0x01);
    if (!dev) {
        printk("AHCI: No SATA AHCI controller present\n");
        return;
    }

    pci_enable_bus_master(dev);
    uint32_t abar_phys = dev->bar[5] & 0xFFFFFFF0U;
    if (!abar_phys) return;

    abar = (volatile uint8_t*)vmm_ioremap(abar_phys, 0x2000U);
    if (!abar) {
        printk("AHCI: Failed to ioremap ABAR 0x%x\n", abar_phys);
        return;
    }

    uint32_t ghc = ahci_read32(HBA_GHC);
    ghc |= HBA_GHC_AE;
    ahci_write32(HBA_GHC, ghc);

    uint32_t vs = ahci_read32(HBA_VS);
    uint32_t pi = ahci_read32(HBA_PI);
    ahci_present = 1;
    printk("AHCI: Controller at %02x:%02x.%u ABAR 0x%x (ver %u.%u, PI 0x%x)\n",
           dev->bus, dev->slot, dev->func, abar_phys,
           (vs >> 16) & 0xFFFFU, vs & 0xFFFFU, pi);

    for (uint8_t p = 0; p < AHCI_MAX_PORTS; p++) {
        if (pi & (1U << p)) {
            ahci_probe_port(p);
        }
    }
}

int ahci_is_present(void) { return ahci_present; }
uint32_t ahci_get_port_count(void) { return active_ports; }

int ahci_read_sectors(uint8_t port_idx, uint64_t lba, uint16_t count, uint8_t *buf) {
    if (!ahci_present || port_idx >= active_ports || !buf || count == 0 || count > 8U) {
        return -1;
    }
    ahci_port_info_t *info = &ahci_ports[port_idx];
    if (!info->present) return -1;

    uint32_t bytes = (uint32_t)count * 512U;
    if (ahci_exec_cmd(info, ATA_CMD_READ_DMA_EXT, lba, count, bytes) != 0) {
        return -1;
    }
    memcpy(buf, info->dma_buf_virt, bytes);
    return 0;
}

void ahci_list_ports(void) {
    if (!ahci_present) {
        printk("AHCI: Controller not present\n");
        return;
    }
    printk("IDX  PORT  MODEL                                    SECTORS     SIZE\n");
    printk("---  ----  ---------------------------------------  ----------  ----\n");
    for (uint32_t i = 0; i < active_ports; i++) {
        printk("%-4u %-5u %-40s %-10u %u MB\n",
               i, ahci_ports[i].port_num, ahci_ports[i].model,
               (uint32_t)ahci_ports[i].sectors,
               (uint32_t)(ahci_ports[i].sectors / 2048ULL));
    }
    if (active_ports == 0) {
        printk("(no attached SATA drives)\n");
    }
}
