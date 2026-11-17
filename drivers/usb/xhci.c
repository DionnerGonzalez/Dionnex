/* ============================================================================
 * Dionnex Monolithic Kernel - USB xHCI (USB 3.x) Host Controller Driver
 * Adapted from Linux (drivers/usb/host/xhci.c, GPL-2.0) & FreeBSD (sys/dev/usb/controller/xhci.c, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/usb/usb.h>
#include <kernel/printk.h>
#include <mm/vmm.h>
#include <mm/dma.h>
#include <string.h>
#include <io.h>

#define XHCI_USBCMD_RS    (1U << 0)
#define XHCI_USBCMD_HCRST (1U << 1)
#define XHCI_USBSTS_CNR   (1U << 11)

int xhci_probe(pci_device_t *dev) {
    if (!dev) return -1;
    uint32_t bar0 = dev->bar[0] & 0xFFFFFFF0U;
    if (!bar0) return -1;

    pci_enable_bus_master(dev);
    volatile uint8_t *mmio = (volatile uint8_t*)vmm_ioremap(bar0, 0x4000U);
    if (!mmio) return -1;

    uint8_t caplen = mmio[0];
    uint16_t hci_ver = *(volatile uint16_t*)(mmio + 2U);
    uint32_t hcsparams1 = *(volatile uint32_t*)(mmio + 0x04U);
    uint8_t max_slots = (uint8_t)(hcsparams1 & 0xFFU);
    uint8_t max_ports = (uint8_t)((hcsparams1 >> 24) & 0xFFU);

    volatile uint32_t *op = (volatile uint32_t*)(mmio + caplen);

    for (uint32_t i = 0; i < 500000U; i++) {
        if (!(op[1] & XHCI_USBSTS_CNR)) break;
        cpu_pause();
    }

    op[0] &= ~XHCI_USBCMD_RS;
    op[0] |= XHCI_USBCMD_HCRST;
    for (uint32_t i = 0; i < 500000U; i++) {
        if (!(op[0] & XHCI_USBCMD_HCRST) && !(op[1] & XHCI_USBSTS_CNR)) break;
        cpu_pause();
    }

    dma_buffer_t b_dcbaa;
    if (dma_alloc_coherent(4096U, &b_dcbaa) == 0) {
        op[0x38 / 4U] = max_slots; /* CONFIG */
        op[0x30 / 4U] = b_dcbaa.phys; /* DCBAAP low */
        op[0x34 / 4U] = 0;
    }

    uint8_t connected = 0;
    for (uint8_t p = 0; p < max_ports; p++) {
        uint32_t portsc = *(volatile uint32_t*)(mmio + caplen + 0x400U + ((uint32_t)p * 0x10U));
        if (portsc & 1U) connected++;
    }

    usb_controller_t hc;
    memset(&hc, 0, sizeof(hc));
    hc.type = USB_HC_XHCI;
    hc.bus = dev->bus;
    hc.slot = dev->slot;
    hc.func = dev->func;
    hc.base_addr = bar0;
    hc.num_ports = max_ports;
    hc.ports_connected = connected;
    strcpy(hc.name, "xHCI");
    usb_register_controller(&hc);

    printk("USB: xHCI v%x.%x at %02x:%02x.%u MMIO 0x%x (%u slots, %u ports, %u connected)\n",
           (hci_ver >> 8) & 0xFFU, hci_ver & 0xFFU,
           dev->bus, dev->slot, dev->func, bar0, max_slots, max_ports, connected);
    return 0;
}
