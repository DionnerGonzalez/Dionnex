/*
 * Dionnex Counter Module
 * Adapted from Linux kernel module style (GPLv2)
 */

extern void printk(const char *fmt, ...);
extern unsigned int timer_get_ticks(void);

static unsigned int start_tick = 0;

int module_init(void) {
    start_tick = timer_get_ticks();
    printk("[counter] Counter module loaded at tick %u\n", start_tick);
    printk("[counter] Use 'rmmod counter' to see elapsed time\n");
    return 0;
}

void module_cleanup(void) {
    unsigned int elapsed = timer_get_ticks() - start_tick;
    printk("[counter] Module was active for %u ticks (%u seconds)\n", elapsed, elapsed / 1000);
}
