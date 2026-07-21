/* Dionnex Kernel Hardware RAID Controller Driver */
#include <drivers/drivers.h>

int hw_raid_init(void) { return 0; }

int storage_raid_init(void)
{
	hw_raid_init();
	return 0;
}
