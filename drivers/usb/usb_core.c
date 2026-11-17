/* ============================================================================
 * Dionnex Monolithic Kernel - USB Host Controller Core Registry
 * Adapted from Linux (drivers/usb/host/, GPL-2.0) & FreeBSD (sys/dev/usb/controller/, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/usb/usb.h>
#include <drivers/pci.h>
#include <kernel/printk.h>
#include <string.h>

static usb_controller_t usb_controllers[USB_MAX_CONTROLLERS];
static uint32_t usb_controller_count = 0;

void usb_register_controller(const usb_controller_t *hc) {
    if (!hc || usb_controller_count >= USB_MAX_CONTROLLERS) return;
    memcpy(&usb_controllers[usb_controller_count], hc, sizeof(usb_controller_t));
    usb_controllers[usb_controller_count].used = 1;
    usb_controller_count++;
}

void usb_init(void) {
    usb_controller_count = 0;
    memset(usb_controllers, 0, sizeof(usb_controllers));

    pci_device_t *curr = pci_get_list();
    while (curr) {
        if (curr->class_code == 0x0C && curr->subclass == 0x03) {
            if (curr->prog_if == 0x00) {
                uhci_probe(curr);
            } else if (curr->prog_if == 0x10) {
                ohci_probe(curr);
            } else if (curr->prog_if == 0x20) {
                ehci_probe(curr);
            } else if (curr->prog_if == 0x30) {
                xhci_probe(curr);
            }
        }
        curr = curr->next;
    }

    printk("USB: Initialized %u host controller(s)\n", usb_controller_count);
}

uint32_t usb_get_controller_count(void) {
    return usb_controller_count;
}

void usb_list_controllers(void) {
    printk("IDX  TYPE   BDF       BASE_ADDR   PORTS  CONNECTED\n");
    printk("---  -----  --------  ----------  -----  ---------\n");
    for (uint32_t i = 0; i < usb_controller_count; i++) {
        usb_controller_t *hc = &usb_controllers[i];
        printk("%-4u %-6s %02x:%02x.%u  0x%08x  %-5u  %u\n",
               i, hc->name, hc->bus, hc->slot, hc->func,
               hc->base_addr, hc->num_ports, hc->ports_connected);
    }
    if (usb_controller_count == 0) {
        printk("(no USB host controllers found)\n");
    }
}
