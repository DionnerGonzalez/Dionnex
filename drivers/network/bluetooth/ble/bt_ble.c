/* Dionnex Kernel Bluetooth Low Energy (BLE) Controller */
#include <drivers/drivers.h>

int bt_ble_init(void) { return 0; }

int net_bluetooth_init(void)
{
	bt_ble_init();
	return 0;
}
