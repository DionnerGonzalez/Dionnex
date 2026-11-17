/* ============================================================================
 * Dionnex Monolithic Kernel - Main Initialization Entry Point
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <types.h>
#include <config.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/klog.h>
#include <kernel/cpuid.h>
#include <kernel/syscall.h>
#include <kernel/elf.h>
#include <kernel/module.h>
#include <kernel/debug.h>
#include <kernel/smp.h>
#include <kernel/shell.h>
#include <kernel/multiboot.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <drivers/rtc.h>
#include <drivers/pci.h>
#include <drivers/pcie.h>
#include <drivers/hpet.h>
#include <drivers/ata.h>
#include <drivers/ahci.h>
#include <drivers/nvme.h>
#include <drivers/usb/usb.h>
#include <drivers/mouse.h>
#include <drivers/apic.h>
#include <drivers/acpi.h>
#include <drivers/vesa.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/task.h>
#include <sched/scheduler.h>
#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/procfs.h>
#include <fs/devfs.h>
#include <fs/fat32.h>
#include <kernel/tss.h>
#include <kernel/process.h>
#include <kernel/scheduler_process.h>
#include <kernel/initrd.h>
#include <kernel/module_loader.h>
#include <kernel/selftest.h>
#include <dionnex_rust.h>
#include <string.h>

static int fat32_ahci_reader(uint64_t lba, uint16_t count, uint8_t *buf) {
    return ahci_read_sectors(0, lba, count, buf);
}

static int fat32_nvme_reader(uint64_t lba, uint16_t count, uint8_t *buf) {
    return nvme_read_sectors(lba, count, buf);
}

static int fat32_ata_reader(uint64_t lba, uint16_t count, uint8_t *buf) {
    return ata_read_sectors(0, 0, (uint32_t)lba, (uint8_t)count, buf);
}

static int try_mount_fat32_device(fat32_read_sector_fn reader) {
    if (fat32_mount(reader, 0) == 0) {
        return 0;
    }
    uint8_t mbr[512];
    if (reader(0, 1U, mbr) == 0 && mbr[510] == 0x55U && mbr[511] == 0xAAU) {
        for (int p = 0; p < 4; p++) {
            const uint8_t *ent = mbr + 446 + (p * 16);
            uint8_t ptype = ent[4];
            uint32_t start_lba = *(const uint32_t*)(ent + 8);
            if ((ptype == 0x0BU || ptype == 0x0CU || ptype == 0x0EU) && start_lba > 0) {
                if (fat32_mount(reader, start_lba) == 0) {
                    return 0;
                }
            }
        }
    }
    return -1;
}

void kernel_main(uint32_t magic, uint32_t mbi_addr) {
    /* 1. Core Early Consoles & Logging */
    vga_init();
    serial_init();
    klog_init();
    pr_info("Dionnex Monolithic Kernel v%s\n", DIONNEX_VERSION);

    /* 2. Multiboot Verification */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kernel_panic("Invalid Multiboot magic signature!");
    }
    multiboot_info_t *mb_info = (multiboot_info_t*)(uintptr_t)mbi_addr;

    /* 3. CPU Tables & Identification */
    gdt_init();
    idt_init();
    cpuid_init();

    /* 4. Memory Management (must precede MMIO/ACPI/PCI/driver init) */
    pmm_init(mb_info);
    vmm_init();
    heap_init(0, DIONNEX_HEAP_SIZE_DEFAULT);

    /* 4b. Safe Rust no_std Kernel Subsystems */
    if (dionnex_rust_init() == 0) {
        pr_info("Rust: %s initialized (selftest PASS)\n", dionnex_rust_version());
    } else {
        kernel_panic("Rust no_std subsystem initialization failed!");
    }

    /* 5. Platform Timers, Interrupts & ACPI */
    acpi_init();
    apic_init();
    hpet_init();
    timer_init(1000);
    keyboard_init();
    mouse_init();
    rtc_init();
    smp_init();

    /* 6. Buses, Storage & USB Hardware */
    pci_init();
    pcie_init();
    ata_init();
    ahci_init();
    nvme_init();
    usb_init();

    /* 7. Filesystems */
    vfs_init();
    ramfs_init();
    vfs_mount("/ram", ramfs_get_root_node());
    procfs_init();
    vfs_mount("/proc", procfs_get_root_node());
    devfs_init();
    vfs_mount("/dev", devfs_get_root_node());

    if (ahci_get_port_count() > 0 && try_mount_fat32_device(fat32_ahci_reader) == 0) {
        vfs_mount("/fat", fat32_get_root_node());
    } else if (nvme_is_present() && try_mount_fat32_device(fat32_nvme_reader) == 0) {
        vfs_mount("/fat", fat32_get_root_node());
    } else if (ata_get_drive_count() > 0 && try_mount_fat32_device(fat32_ata_reader) == 0) {
        vfs_mount("/fat", fat32_get_root_node());
    }

    /* 8. Scheduling, Syscalls & Userspace */
    task_init();
    scheduler_init();
    syscall_init();
    tss_init();
    process_init();
    sched_proc_init();
    initrd_init();
    module_loader_init();
    elf_init();
    vesa_init(mb_info);
    module_init_subsystem();

    /* 9. Populate RAMFS welcome files */
    ramfs_create("welcome.txt");
    ramfs_write("welcome.txt", (const uint8_t*)"Welcome to Dionnex v0.2\n", 24);
    ramfs_create("motd.txt");
    ramfs_write("motd.txt", (const uint8_t*)"Dionnex Monolithic Kernel\nGPLv2\nType 'help' for commands.\n", 57);

    /* 10. Load built-in relocatable kernel module (.ko) */
    uint32_t hello_sz = 0;
    uint8_t *hello_data = initrd_get_file("hello.ko", &hello_sz);
    if (hello_data && hello_sz > 0) {
        module_load_binary("hello", hello_data, hello_sz);
    }

    /* 11. Enable Hardware Interrupts */
    asm volatile ("sti");

    /* 12. Execute Ring 3 Init Self-Verification Process */
    uint32_t init_sz = 0;
    uint8_t *init_data = initrd_get_file("init.elf", &init_sz);
    if (init_data && init_sz > 0) {
        process_t *init_proc = process_create("init", init_data, init_sz);
        if (init_proc) {
            int exit_code = process_run(init_proc);
            pr_info("Init: Process '%s' (pid %u) completed with exit code %d\n",
                    init_proc->name, init_proc->pid, exit_code);
            process_destroy(init_proc);
        }
    }

    pr_info("All subsystems initialized\n");
    printk("\n  Welcome to Dionnex v%s (%s)\n", DIONNEX_VERSION, DIONNEX_CODENAME);
    printk("  Type 'help' for available commands.\n\n");

    /* 13. Interactive Kernel Shell */
    shell_run();
}
