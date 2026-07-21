/*
 * Dionnex Kernel PCI Power Management
 */

#include <drivers/drivers.h>

int pci_pm_init(void)
{
	return 0;
}

int pci_subsystem_init(void)
{
	pci_pm_init();
	return 0;
}
