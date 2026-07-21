/* Dionnex Kernel Intel Performance Monitoring Unit (PMU) Driver */
#include <drivers/drivers.h>

int intel_pmu_init(void) { return 0; }

int cpu_intel_init(void)
{
	intel_pmu_init();
	return 0;
}
