/* ============================================================================
 * Dionnex Monolithic Kernel - USB UHCI Host Controller Driver
 * Adapted from Linux (drivers/usb/host/uhci-hcd.c, GPL-2.0) & FreeBSD (sys/dev/usb/controller/uhci.c, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/usb/usb.h>
#include <kernel/printk.h>
#include <mm/dma.h>
#include <string.h>
#include <io.h>

#define UHCI_USBCMD      0x00U
#define UHCI_USBSTS      0x02U
#define UHCI_FRBASEADD   0x08U
#define UHCI_PORTSC1     0x10U
#define UHCI_PORTSC2     0x12U

#define UHCI_CMD_HCRESET (1U << 1)
#define UHCI_CMD_RS      (1U << 0)

int uhci_probe(pci_device_t *dev) {
    if (!dev) return -1;
    uint32_t io_base = dev->bar[4] & 0xFFFFFFFCU;
    if (io_base == 0) return -1;

    pci_enable_bus_master(dev);

    outw((uint16_t)(io_base + UHCI_USBCMD), UHCI_CMD_HCRESET);
    for (uint32_t i = 0; i < 100000U; i++) {
        if (!(inw((uint16_t)(io_base + UHCI_USBCMD)) & UHCI_CMD_HCRESET)) break;
        cpu_pause();
    }

    dma_buffer_t b_fl;
    if (dma_alloc_coherent(4096U, &b_fl) == 0) {
        uint32_t *frame_list = (uint32_t*)b_fl.virt;
        for (int i = 0; i < 1024; i++) {
            frame_list[i] = 1U; /* Terminate bit */
        }
        outl((uint16_t)(io_base + UHCI_FRBASEADD), b_fl.phys);
        outw((uint16_t)(io_base + UHCI_USBCMD), UHCI_CMD_RS);
    }

    uint16_t p1 = inw((uint16_t)(io_base + UHCI_PORTSC1));
    uint16_t p2 = inw((uint16_t)(io_base + UHCI_PORTSC2));
    uint8_t connected = (uint8_t)(((p1 & 1U) ? 1 : 0) + ((p2 & 1U) ? 1 : 0));

    usb_controller_t hc;
    memset(&hc, 0, sizeof(hc));
    hc.type = USB_HC_UHCI;
    hc.bus = dev->bus;
    hc.slot = dev->slot;
    hc.func = dev->func;
    hc.base_addr = io_base;
    hc.num_ports = 2;
    hc.ports_connected = connected;
    strcpy(hc.name, "UHCI");
    usb_register_controller(&hc);

    printk("USB: UHCI at %02x:%02x.%u I/O 0x%x (2 ports, %u connected)\n",
           dev->bus, dev->slot, dev->func, io_base, connected);
    return 0;
}
