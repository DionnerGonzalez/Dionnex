/*
 * Dionnex System Info Module
 * Adapted from Linux kernel module style (GPLv2)
 */

extern void printk(const char *fmt, ...);
extern unsigned int pmm_get_total_count(void);
extern unsigned int pmm_get_free_count(void);
extern unsigned int pci_get_count(void);
extern unsigned int process_get_count(void);

int module_init(void) {
    printk("[sysinfo] === Dionnex System Info Module ===\n");
    printk("[sysinfo] Memory: %u KB total, %u KB free\n", pmm_get_total_count() * 4, pmm_get_free_count() * 4);
    printk("[sysinfo] PCI devices: %u\n", pci_get_count());
    printk("[sysinfo] User processes: %u\n", process_get_count());
    printk("[sysinfo] ================================\n");
    return 0;
}

void module_cleanup(void) {
    printk("[sysinfo] System info module unloaded\n");
}
