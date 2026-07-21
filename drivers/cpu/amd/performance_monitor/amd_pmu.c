/* Dionnex Kernel AMD Core Performance Monitor */
#include <drivers/drivers.h>

int amd_pmu_init(void) { return 0; }

int cpu_amd_init(void)
{
	amd_pmu_init();
	return 0;
}
