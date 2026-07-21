/* Dionnex Kernel Ultra-Low-Power IoT Device Driver */
#include <drivers/drivers.h>

int iot_driver_init(void) { return 0; }

int embedded_subsystem_init(void)
{
	iot_driver_init();
	return 0;
}
