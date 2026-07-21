/* Dionnex Kernel Embedded System Co-Processor Firmware Manager */
#include <drivers/drivers.h>

int fw_embedded_loader_init(void) { return 0; }

int firmware_subsystem_init(void)
{
	fw_embedded_loader_init();
	return 0;
}
