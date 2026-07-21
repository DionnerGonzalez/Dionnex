/* Dionnex Kernel Intel Arc Discrete GPU Driver */
#include <drivers/drivers.h>

int intel_arc_init(void) { return 0; }

int gpu_intel_init(void)
{
	intel_arc_init();
	return 0;
}
