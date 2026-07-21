/* Dionnex Kernel Serial ATA AHCI Controller Driver */
#include <drivers/drivers.h>

int ahci_init(void) { return 0; }

int storage_sata_init(void)
{
	ahci_init();
	return 0;
}
