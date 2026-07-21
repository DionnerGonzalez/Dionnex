/* Dionnex Kernel ARM Custom SoC Platform Driver */
#include <drivers/drivers.h>

int arm_soc_init(void) { return 0; }

int cpu_arm_init(void)
{
	arm_soc_init();
	return 0;
}
