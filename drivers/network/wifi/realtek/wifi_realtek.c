/* Dionnex Kernel Realtek RTL88xx Wi-Fi Driver */
#include <drivers/drivers.h>

int wifi_realtek_init(void) { return 0; }

int net_wifi_init(void)
{
	wifi_realtek_init();
	return 0;
}
