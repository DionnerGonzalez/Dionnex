/* Dionnex Kernel Block Storage Layer Core */
#include <drivers/drivers.h>

int block_core_init(void) { return 0; }

int storage_block_init(void)
{
	block_core_init();
	return 0;
}
