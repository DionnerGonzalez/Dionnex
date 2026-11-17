/* ============================================================================
 * Dionnex Monolithic Kernel - USB OHCI Host Controller Driver
 * Adapted from Linux (drivers/usb/host/ohci-hcd.c, GPL-2.0) & FreeBSD (sys/dev/usb/controller/ohci.c, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/usb/usb.h>
#include <kernel/printk.h>
#include <mm/vmm.h>
#include <mm/dma.h>
#include <string.h>
#include <io.h>

#define OHCI_CTRL        0x04U
#define OHCI_CMD_STATUS  0x08U
#define OHCI_HCCA        0x18U
#define OHCI_RH_DESCA    0x48U
#define OHCI_RH_PORTSTAT 0x54U

int ohci_probe(pci_device_t *dev) {
    if (!dev) return -1;
    uint32_t bar0 = dev->bar[0] & 0xFFFFFFF0U;
    if (!bar0) return -1;

    pci_enable_bus_master(dev);
    volatile uint32_t *regs = (volatile uint32_t*)vmm_ioremap(bar0, 0x1000U);
    if (!regs) return -1;

    regs[OHCI_CMD_STATUS / 4U] = 1U; /* HCR reset */
    for (uint32_t i = 0; i < 100000U; i++) {
        if (!(regs[OHCI_CMD_STATUS / 4U] & 1U)) break;
        cpu_pause();
    }

    dma_buffer_t b_hcca;
    if (dma_alloc_coherent(4096U, &b_hcca) == 0) {
        regs[OHCI_HCCA / 4U] = b_hcca.phys;
    }

    uint8_t num_ports = (uint8_t)(regs[OHCI_RH_DESCA / 4U] & 0xFFU);
    if (num_ports > 15U) num_ports = 15U;
    uint8_t connected = 0;
    for (uint8_t p = 0; p < num_ports; p++) {
        if (regs[(OHCI_RH_PORTSTAT / 4U) + p] & 1U) connected++;
    }

    usb_controller_t hc;
    memset(&hc, 0, sizeof(hc));
    hc.type = USB_HC_OHCI;
    hc.bus = dev->bus;
    hc.slot = dev->slot;
    hc.func = dev->func;
    hc.base_addr = bar0;
    hc.num_ports = num_ports;
    hc.ports_connected = connected;
    strcpy(hc.name, "OHCI");
    usb_register_controller(&hc);

    printk("USB: OHCI at %02x:%02x.%u MMIO 0x%x (%u ports, %u connected)\n",
           dev->bus, dev->slot, dev->func, bar0, num_ports, connected);
    return 0;
}
