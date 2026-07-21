/* Dionnex Kernel RISC-V Hardware Performance Monitor (HPM/PMU) Driver */
#include <drivers/drivers.h>

int riscv_pmu_init(void) { return 0; }

int cpu_riscv_init(void)
{
	riscv_pmu_init();
	return 0;
}
