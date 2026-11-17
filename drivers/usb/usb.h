/* ============================================================================
 * Dionnex Monolithic Kernel - Unified USB Host Controller Interface Header
 * Adapted from Linux (drivers/usb/host/, GPL-2.0) & FreeBSD (sys/dev/usb/controller/, BSD-2-Clause)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_USB_H_
#define _DIONNEX_USB_H_

#include <types.h>
#include <drivers/pci.h>

#define USB_MAX_CONTROLLERS 16

typedef enum usb_hc_type {
    USB_HC_NONE = 0,
    USB_HC_UHCI = 1,
    USB_HC_OHCI = 2,
    USB_HC_EHCI = 3,
    USB_HC_XHCI = 4
} usb_hc_type_t;

typedef struct usb_controller {
    uint8_t       used;
    usb_hc_type_t type;
    uint8_t       bus;
    uint8_t       slot;
    uint8_t       func;
    uint32_t      base_addr;
    uint8_t       num_ports;
    uint8_t       ports_connected;
    char          name[16];
} usb_controller_t;

void usb_init(void);
void usb_register_controller(const usb_controller_t *hc);
uint32_t usb_get_controller_count(void);
void usb_list_controllers(void);

int uhci_probe(pci_device_t *dev);
int ohci_probe(pci_device_t *dev);
int ehci_probe(pci_device_t *dev);
int xhci_probe(pci_device_t *dev);

#endif /* _DIONNEX_USB_H_ */
