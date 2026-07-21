/* Dionnex Kernel Universal Cellular Modem Manager */
#include <drivers/drivers.h>

int cellular_modem_init(void) { return 0; }

int net_cellular_init(void)
{
	cellular_modem_init();
	return 0;
}
