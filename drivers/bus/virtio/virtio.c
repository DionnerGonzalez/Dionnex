/* Dionnex Kernel VirtIO Para-Virtualization Bus Driver */
#include <drivers/drivers.h>

int virtio_bus_init(void) { return 0; }

int bus_peripherals_init(void)
{
	virtio_bus_init();
	return 0;
}
