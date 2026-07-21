/* Dionnex Kernel Sound Subsystem Core (PCM, Mixer, ALSA/OSS KABI) */
#include <drivers/drivers.h>

int audio_core_init(void) { return 0; }

int audio_subsystem_init(void)
{
	audio_core_init();
	return 0;
}
