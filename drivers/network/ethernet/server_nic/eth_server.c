/* Dionnex Kernel Enterprise Multi-Port Server NIC Driver */
#include <drivers/drivers.h>

int eth_server_init(void) { return 0; }

int net_ethernet_init(void)
{
	eth_server_init();
	return 0;
}
