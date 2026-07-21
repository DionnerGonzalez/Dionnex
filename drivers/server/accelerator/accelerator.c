/* Dionnex Kernel AI / NPU Hardware Accelerator Driver (TPU, Gaudi, Groq) */
#include <drivers/drivers.h>

int accelerator_driver_init(void) { return 0; }

int server_subsystem_init(void)
{
	accelerator_driver_init();
	return 0;
}
