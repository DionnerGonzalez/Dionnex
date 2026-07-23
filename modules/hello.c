/*
 * Dionnex Hello Module
 * Adapted from Linux kernel module style (GPLv2)
 */

extern void printk(const char *fmt, ...);
extern void *kmalloc(unsigned int size);
extern void kfree(void *ptr);
extern unsigned int timer_get_ticks(void);

static int hello_count = 0;

int module_init(void) {
    printk("[hello] Hello from Dionnex kernel module!\n");
    printk("[hello] Module loaded at tick %u\n", timer_get_ticks());
    printk("[hello] Kernel memory available\n");
    hello_count = 1;
    return 0;
}

void module_cleanup(void) {
    printk("[hello] Goodbye from Dionnex kernel module!\n");
    printk("[hello] Module was active for %u ticks\n", timer_get_ticks());
    hello_count = 0;
}
