/* Dionnex Kernel ARM Mali GPU Driver */
#include <drivers/drivers.h>

int arm_mali_init(void) { return 0; }

int gpu_arm_init(void)
{
	arm_mali_init();
	return 0;
}
