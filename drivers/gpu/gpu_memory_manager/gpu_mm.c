/* Dionnex Kernel GPU VRAM & Memory Allocation Manager */
#include <drivers/drivers.h>

int gpu_mm_init(void) { return 0; }

int gpu_display_init(void)
{
	gpu_mm_init();
	return 0;
}
