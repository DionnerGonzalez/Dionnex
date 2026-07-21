/* Dionnex Kernel NVIDIA CUDA / Tensor Compute Engine */
#include <drivers/drivers.h>

int nv_compute_init(void) { return 0; }

int gpu_nvidia_init(void)
{
	nv_compute_init();
	return 0;
}
