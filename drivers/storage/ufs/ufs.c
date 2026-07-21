/* Dionnex Kernel Universal Flash Storage (UFS) Driver */
#include <drivers/drivers.h>

int ufs_driver_init(void) { return 0; }

int storage_mobile_media_init(void)
{
	ufs_driver_init();
	return 0;
}
