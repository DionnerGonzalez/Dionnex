/* Dionnex Kernel USB Video Class (UVC) Webcam Driver */
#include <drivers/drivers.h>

int webcam_driver_init(void) { return 0; }

int camera_subsystem_init(void)
{
	webcam_driver_init();
	return 0;
}
