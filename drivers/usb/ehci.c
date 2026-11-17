/* ============================================================================
 * Dionnex Monolithic Kernel - USB EHCI Host Controller Driver
 * Adapted from Linux (drivers/usb/host/ehci-hcd.c, GPL-2.0) & FreeBSD (sys/dev/usb/controller/ehci.c, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/usb/usb.h>
#include <kernel/printk.h>
#include <mm/vmm.h>
#include <string.h>
#include <io.h>

int ehci_probe(pci_device_t *dev) {
    if (!dev) return -1;
    uint32_t bar0 = dev->bar[0] & 0xFFFFFFF0U;
    if (!bar0) return -1;

    pci_enable_bus_master(dev);
    volatile uint8_t *mmio = (volatile uint8_t*)vmm_ioremap(bar0, 0x1000U);
    if (!mmio) return -1;

    uint8_t caplen = mmio[0];
    uint32_t hcsparams = *(volatile uint32_t*)(mmio + 0x04U);
    uint8_t num_ports = (uint8_t)(hcsparams & 0x0FU);

    volatile uint32_t *op = (volatile uint32_t*)(mmio + caplen);
    op[0] |= (1U << 1); /* USBCMD HCRESET */
    for (uint32_t i = 0; i < 200000U; i++) {
        if (!(op[0] & (1U << 1))) break;
        cpu_pause();
    }
    op[0x40 / 4U] = 1U; /* CONFIGFLAG = 1 */

    uint8_t connected = 0;
    for (uint8_t p = 0; p < num_ports; p++) {
        uint32_t portsc = op[(0x44U / 4U) + p];
        if (portsc & 1U) connected++;
    }

    usb_controller_t hc;
    memset(&hc, 0, sizeof(hc));
    hc.type = USB_HC_EHCI;
    hc.bus = dev->bus;
    hc.slot = dev->slot;
    hc.func = dev->func;
    hc.base_addr = bar0;
    hc.num_ports = num_ports;
    hc.ports_connected = connected;
    strcpy(hc.name, "EHCI");
    usb_register_controller(&hc);

    printk("USB: EHCI at %02x:%02x.%u MMIO 0x%x (%u ports, %u connected)\n",
           dev->bus, dev->slot, dev->func, bar0, num_ports, connected);
    return 0;
}
