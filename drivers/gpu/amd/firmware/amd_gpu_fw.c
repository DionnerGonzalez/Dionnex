/* Dionnex Kernel AMD GPU Microcode & Firmware Loader */
#include <drivers/drivers.h>

int amd_gpu_fw_init(void) { return 0; }

int gpu_amd_init(void)
{
	amd_gpu_fw_init();
	return 0;
}
