/*
 * Dionnex Kernel PCIe Advanced Features
 */

#include <drivers/drivers.h>

int pcie_adv_init(void)
{
	return 0;
}

int pcie_subsystem_init(void)
{
	pcie_adv_init();
	return 0;
}
