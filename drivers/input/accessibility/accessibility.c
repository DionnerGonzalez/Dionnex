/* Dionnex Kernel Assistive Input & Switch Device Driver */
#include <drivers/drivers.h>

int accessibility_driver_init(void) { return 0; }

int input_devices_init(void)
{
	accessibility_driver_init();
	return 0;
}
