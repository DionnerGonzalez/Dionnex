/* Dionnex Kernel NVMe Controller Core Driver */
#include <drivers/drivers.h>

int nvme_ctrl_init(void) { return 0; }

int storage_nvme_init(void)
{
	nvme_ctrl_init();
	return 0;
}
