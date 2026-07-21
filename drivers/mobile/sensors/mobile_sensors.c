/* Dionnex Kernel Mobile IMU, Gyroscope, Accelerometer, Barometer Sensors Driver */
#include <drivers/drivers.h>

int mobile_sensors_init(void) { return 0; }

int mobile_subsystem_init(void)
{
	mobile_sensors_init();
	return 0;
}
