/* ============================================================================
 * Dionnex Monolithic Kernel - NVMe PCIe Storage Controller Driver
 * Adapted from Linux (drivers/nvme/host/pci.c, GPL-2.0) & FreeBSD (sys/dev/nvme/, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/nvme.h>
#include <drivers/pci.h>
#include <kernel/printk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/dma.h>
#include <string.h>
#include <io.h>

#define NVME_REG_CAP    0x0000U
#define NVME_REG_VS     0x0008U
#define NVME_REG_CC     0x0014U
#define NVME_REG_CSTS   0x001CU
#define NVME_REG_AQA    0x0024U
#define NVME_REG_ASQ    0x0028U
#define NVME_REG_ACQ    0x0030U

#define NVME_CC_EN      (1U << 0)
#define NVME_CSTS_RDY   (1U << 0)

#define NVME_ADMIN_CREATE_SQ 0x01U
#define NVME_ADMIN_CREATE_CQ 0x05U
#define NVME_ADMIN_IDENTIFY  0x06U
#define NVME_NVM_READ        0x02U

#define NVME_Q_DEPTH 64U

typedef struct nvme_sqe {
    uint32_t cdw0;
    uint32_t nsid;
    uint32_t rsv2[2];
    uint64_t mptr;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
} __attribute__((packed)) nvme_sqe_t;

typedef struct nvme_cqe {
    uint32_t result;
    uint32_t rsv;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;
} __attribute__((packed)) nvme_cqe_t;

typedef struct nvme_queue {
    nvme_sqe_t *sq;
    nvme_cqe_t *cq;
    uint32_t sq_phys;
    uint32_t cq_phys;
    uint16_t sq_tail;
    uint16_t cq_head;
    uint8_t  cq_phase;
    uint16_t qid;
} nvme_queue_t;

static volatile uint8_t *nvme_regs = NULL;
static uint32_t nvme_dstrd = 4U;
static nvme_queue_t admin_q;
static nvme_queue_t io_q;
static void *nvme_dma_buf = NULL;
static uint32_t nvme_dma_phys = 0;

static int nvme_ready = 0;
static char nvme_model[41];
static char nvme_serial[21];
static uint64_t nvme_ns_sectors = 0;
static uint32_t nvme_sector_size = 512U;
static uint16_t nvme_cid = 1U;

static inline uint32_t nvme_read32(uint32_t off) {
    return *(volatile uint32_t*)(nvme_regs + off);
}

static inline void nvme_write32(uint32_t off, uint32_t val) {
    *(volatile uint32_t*)(nvme_regs + off) = val;
}

static inline void nvme_write64(uint32_t off, uint64_t val) {
    nvme_write32(off, (uint32_t)(val & 0xFFFFFFFFULL));
    nvme_write32(off + 4U, (uint32_t)(val >> 32));
}

static void nvme_ring_sq_doorbell(uint16_t qid, uint16_t tail) {
    uint32_t off = 0x1000U + ((uint32_t)(2U * qid) * nvme_dstrd);
    nvme_write32(off, tail);
}

static void nvme_ring_cq_doorbell(uint16_t qid, uint16_t head) {
    uint32_t off = 0x1000U + ((uint32_t)(2U * qid + 1U) * nvme_dstrd);
    nvme_write32(off, head);
}

static int nvme_submit_sync(nvme_queue_t *q, const nvme_sqe_t *cmd) {
    uint16_t tail = q->sq_tail;
    memcpy(&q->sq[tail], cmd, sizeof(nvme_sqe_t));
    q->sq[tail].cdw0 = (cmd->cdw0 & 0xFFFFU) | ((uint32_t)(nvme_cid++) << 16);

    q->sq_tail = (uint16_t)((tail + 1U) % NVME_Q_DEPTH);
    nvme_ring_sq_doorbell(q->qid, q->sq_tail);

    for (uint32_t spin = 0; spin < 3000000U; spin++) {
        volatile nvme_cqe_t *cqe = &q->cq[q->cq_head];
        uint16_t status = cqe->status;
        if ((status & 1U) == q->cq_phase) {
            uint16_t code = (status >> 1) & 0x7FFFU;
            q->cq_head++;
            if (q->cq_head >= NVME_Q_DEPTH) {
                q->cq_head = 0;
                q->cq_phase ^= 1U;
            }
            nvme_ring_cq_doorbell(q->qid, q->cq_head);
            return (code == 0) ? 0 : -1;
        }
        cpu_pause();
    }
    return -1;
}

static void nvme_copy_trimmed(char *dst, const uint8_t *src, uint32_t len) {
    memcpy(dst, src, len);
    dst[len] = '\0';
    for (int i = (int)len - 1; i >= 0; i--) {
        if (dst[i] == ' ' || dst[i] == '\0') dst[i] = '\0';
        else break;
    }
}

void nvme_init(void) {
    nvme_ready = 0;
    nvme_ns_sectors = 0;
    memset(&admin_q, 0, sizeof(admin_q));
    memset(&io_q, 0, sizeof(io_q));

    pci_device_t *dev = pci_get_class_progif(0x01, 0x08, 0x02);
    if (!dev) {
        printk("NVMe: No NVMe controller present\n");
        return;
    }

    pci_enable_bus_master(dev);
    uint32_t bar0 = dev->bar[0] & 0xFFFFFFF0U;
    if (!bar0) return;

    nvme_regs = (volatile uint8_t*)vmm_ioremap(bar0, 0x4000U);
    if (!nvme_regs) {
        printk("NVMe: Failed to ioremap BAR0 0x%x\n", bar0);
        return;
    }

    uint32_t cap_hi = nvme_read32(NVME_REG_CAP + 4U);
    uint32_t vs = nvme_read32(NVME_REG_VS);
    nvme_dstrd = 4U << (cap_hi & 0x0FU);

    /* Disable controller before configuring Admin Queues */
    uint32_t cc = nvme_read32(NVME_REG_CC);
    if (cc & NVME_CC_EN) {
        nvme_write32(NVME_REG_CC, cc & ~NVME_CC_EN);
        for (uint32_t i = 0; i < 2000000U; i++) {
            if (!(nvme_read32(NVME_REG_CSTS) & NVME_CSTS_RDY)) break;
            cpu_pause();
        }
    }

    admin_q.qid = 0;
    admin_q.cq_phase = 1;
    dma_buffer_t b_sq, b_cq, b_io;
    if (dma_alloc_coherent(4096U, &b_sq) != 0 ||
        dma_alloc_coherent(4096U, &b_cq) != 0 ||
        dma_alloc_coherent(4096U, &b_io) != 0) {
        return;
    }
    admin_q.sq = (nvme_sqe_t*)b_sq.virt; admin_q.sq_phys = b_sq.phys;
    admin_q.cq = (nvme_cqe_t*)b_cq.virt; admin_q.cq_phys = b_cq.phys;
    nvme_dma_buf = b_io.virt; nvme_dma_phys = b_io.phys;

    nvme_write32(NVME_REG_AQA, ((NVME_Q_DEPTH - 1U) << 16) | (NVME_Q_DEPTH - 1U));
    nvme_write64(NVME_REG_ASQ, admin_q.sq_phys);
    nvme_write64(NVME_REG_ACQ, admin_q.cq_phys);

    /* IOSQES=6 (64 bytes), IOCQES=4 (16 bytes), EN=1 */
    nvme_write32(NVME_REG_CC, (4U << 20) | (6U << 16) | NVME_CC_EN);
    for (uint32_t i = 0; i < 2000000U; i++) {
        if (nvme_read32(NVME_REG_CSTS) & NVME_CSTS_RDY) break;
        cpu_pause();
    }
    if (!(nvme_read32(NVME_REG_CSTS) & NVME_CSTS_RDY)) {
        printk("NVMe: Controller failed to become ready\n");
        return;
    }

    /* Identify Controller (CNS = 1) */
    nvme_sqe_t id_ctrl;
    memset(&id_ctrl, 0, sizeof(id_ctrl));
    id_ctrl.cdw0 = NVME_ADMIN_IDENTIFY;
    id_ctrl.prp1 = nvme_dma_phys;
    id_ctrl.cdw10 = 1U;
    if (nvme_submit_sync(&admin_q, &id_ctrl) != 0) {
        printk("NVMe: Identify Controller failed\n");
        return;
    }

    const uint8_t *ctrl_data = (const uint8_t*)nvme_dma_buf;
    nvme_copy_trimmed(nvme_serial, ctrl_data + 4, 20U);
    nvme_copy_trimmed(nvme_model, ctrl_data + 24, 40U);

    /* Identify Namespace 1 (CNS = 0) */
    nvme_sqe_t id_ns;
    memset(&id_ns, 0, sizeof(id_ns));
    id_ns.cdw0 = NVME_ADMIN_IDENTIFY;
    id_ns.nsid = 1U;
    id_ns.prp1 = nvme_dma_phys;
    id_ns.cdw10 = 0U;
    if (nvme_submit_sync(&admin_q, &id_ns) == 0) {
        const uint8_t *ns_data = (const uint8_t*)nvme_dma_buf;
        nvme_ns_sectors = *(const uint64_t*)(ns_data + 0);
        uint8_t flbas = ns_data[26] & 0x0FU;
        uint32_t lbaf = *(const uint32_t*)(ns_data + 128U + (flbas * 4U));
        uint8_t lbads = (lbaf >> 16) & 0xFFU;
        if (lbads >= 9U && lbads <= 12U) {
            nvme_sector_size = 1U << lbads;
        }
    }

    /* Create I/O CQ 1 and SQ 1 */
    io_q.qid = 1;
    io_q.cq_phase = 1;
    dma_buffer_t io_sq, io_cq;
    if (dma_alloc_coherent(4096U, &io_sq) == 0 &&
        dma_alloc_coherent(4096U, &io_cq) == 0) {
        io_q.sq = (nvme_sqe_t*)io_sq.virt; io_q.sq_phys = io_sq.phys;
        io_q.cq = (nvme_cqe_t*)io_cq.virt; io_q.cq_phys = io_cq.phys;
        nvme_sqe_t ccq;
        memset(&ccq, 0, sizeof(ccq));
        ccq.cdw0 = NVME_ADMIN_CREATE_CQ;
        ccq.prp1 = io_q.cq_phys;
        ccq.cdw10 = ((NVME_Q_DEPTH - 1U) << 16) | 1U;
        ccq.cdw11 = 1U; /* Physically contiguous */
        if (nvme_submit_sync(&admin_q, &ccq) == 0) {
            nvme_sqe_t csq;
            memset(&csq, 0, sizeof(csq));
            csq.cdw0 = NVME_ADMIN_CREATE_SQ;
            csq.prp1 = io_q.sq_phys;
            csq.cdw10 = ((NVME_Q_DEPTH - 1U) << 16) | 1U;
            csq.cdw11 = (1U << 16) | 1U; /* CQID = 1, PC = 1 */
            nvme_submit_sync(&admin_q, &csq);
        }
    }

    nvme_ready = 1;
    uint32_t mb = (uint32_t)((nvme_ns_sectors * (uint64_t)nvme_sector_size) / (1024ULL * 1024ULL));
    printk("NVMe: %s (SN: %s, ver %u.%u, NS1 %u MB, %uB LBA)\n",
           nvme_model, nvme_serial, (vs >> 16) & 0xFFFFU, (vs >> 8) & 0xFFU,
           mb, nvme_sector_size);
}

int nvme_is_present(void) { return nvme_ready; }
uint64_t nvme_get_ns_sectors(void) { return nvme_ns_sectors; }
uint32_t nvme_get_sector_size(void) { return nvme_sector_size; }

int nvme_read_sectors(uint64_t lba, uint16_t count, uint8_t *buf) {
    if (!nvme_ready || !io_q.sq || !buf || count == 0 || count > 8U) return -1;

    nvme_sqe_t rd;
    memset(&rd, 0, sizeof(rd));
    rd.cdw0 = NVME_NVM_READ;
    rd.nsid = 1U;
    rd.prp1 = nvme_dma_phys;
    rd.cdw10 = (uint32_t)(lba & 0xFFFFFFFFULL);
    rd.cdw11 = (uint32_t)(lba >> 32);
    rd.cdw12 = (uint32_t)(count - 1U);

    if (nvme_submit_sync(&io_q, &rd) != 0) {
        return -1;
    }
    memcpy(buf, nvme_dma_buf, (uint32_t)count * nvme_sector_size);
    return 0;
}

void nvme_print_info(void) {
    if (!nvme_ready) {
        printk("NVMe: Controller not present\n");
        return;
    }
    uint32_t mb = (uint32_t)((nvme_ns_sectors * (uint64_t)nvme_sector_size) / (1024ULL * 1024ULL));
    printk("NVMe Controller: %s\n", nvme_model);
    printk("  Serial      : %s\n", nvme_serial);
    printk("  Sector Size : %u bytes\n", nvme_sector_size);
    printk("  NS1 Capacity: %u sectors (%u MB)\n", (uint32_t)nvme_ns_sectors, mb);
}
