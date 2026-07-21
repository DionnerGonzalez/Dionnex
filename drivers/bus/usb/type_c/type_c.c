/*
 * Dionnex Kernel USB 2.0 / USB 3.0 / USB 4 & Type-C Controllers
 */

#include <drivers/drivers.h>

int usb2_init(void) { return 0; }
int usb3_init(void) { return 0; }
int usb4_init(void) { return 0; }
int type_c_init(void) { return 0; }

int usb_subsystem_init(void)
{
	usb2_init();
	usb3_init();
	usb4_init();
	type_c_init();
	return 0;
}
