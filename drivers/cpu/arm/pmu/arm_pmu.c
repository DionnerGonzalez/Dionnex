/* Dionnex Kernel ARM Performance Monitor Unit (PMU) Driver */
#include <drivers/drivers.h>

int arm_pmu_init(void) { return 0; }

int cpu_arm_init(void)
{
	arm_pmu_init();
	return 0;
}
