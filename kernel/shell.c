/* ============================================================================
 * Dionnex Monolithic Kernel - Interactive Kernel Shell Implementation
 * ============================================================================
 */
#include <kernel/shell.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/timer.h>
#include <kernel/cpuid.h>
#include <kernel/klog.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/smp.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <drivers/pci.h>
#include <drivers/ata.h>
#include <drivers/rtc.h>
#include <drivers/acpi.h>
#include <drivers/vesa.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <sched/scheduler.h>
#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <string.h>

static char cmd_buf[256];
static uint32_t cmd_pos = 0;

static void shell_parse(char *buf, char **cmd, char **arg1, char **arg2) {
    *cmd = NULL;
    *arg1 = NULL;
    *arg2 = NULL;

    while (*buf == ' ') buf++;
    if (!*buf) return;

    *cmd = buf;
    while (*buf && *buf != ' ') buf++;

    if (*buf) {
        *buf = '\0';
        buf++;
        while (*buf == ' ') buf++;
        if (*buf) {
            *arg1 = buf;
            while (*buf && *buf != ' ') buf++;
            if (*buf) {
                *buf = '\0';
                buf++;
                while (*buf == ' ') buf++;
                if (*buf) {
                    *arg2 = buf;
                }
            }
        }
    }
}

static void shell_prompt(void) {
    printk("dionnex> ");
}

void shell_init(void) {
    vga_clear();
    printk("\n");
    printk("  ===============================================================\n");
    printk("  Dionnex Monolithic Kernel v0.1.0 (Genesis)\n");
    printk("  x86 32-bit Monolithic Operating System Kernel | GPLv2\n");
    printk("  ===============================================================\n");
    printk("\n");
    printk("  Type 'help' for available commands.\n\n");
}

static void shell_exec(char *buf) {
    char *cmd = NULL;
    char *arg1 = NULL;
    char *arg2 = NULL;

    char orig_copy[256];
    strncpy(orig_copy, buf, 255);
    orig_copy[255] = '\0';

    shell_parse(buf, &cmd, &arg1, &arg2);

    if (!cmd || strlen(cmd) == 0) return;

    if (strcmp(cmd, "help") == 0) {
        printk("Dionnex Monolithic Kernel Shell Commands:\n");
        printk("  help            - Display this command reference\n");
        printk("  clear           - Clear VGA screen buffer\n");
        printk("  cpuid / cpuinfo - Display CPU vendor, model, and feature flags\n");
        printk("  pci             - List all PCI bus devices\n");
        printk("  disk            - List ATA hard disk drives\n");
        printk("  date            - Show Real Time Clock (RTC) date and time\n");
        printk("  dmesg / klog    - Dump ring buffer kernel logs\n");
        printk("  lsmod           - List loaded kernel modules\n");
        printk("  smp             - List SMP CPU cores and topology\n");
        printk("  vesa            - Run VESA VBE graphics demo\n");
        printk("  memstat         - Show physical memory and heap diagnostics\n");
        printk("  backtrace       - Print stack backtrace frame pointers\n");
        printk("  hexdump <a,l>   - Hexdump memory address <hex_addr> <len>\n");
        printk("  ps              - List active scheduler processes\n");
        printk("  ls <path>       - List VFS directory contents\n");
        printk("  cat <file>      - Display file content from VFS\n");
        printk("  echo <msg>      - Print message or write to file (> file)\n");
        printk("  uptime          - Show system uptime in seconds\n");
        printk("  reboot          - Reboot system via ACPI/8042\n");
        printk("  shutdown        - Power off system via ACPI/QEMU\n");
        printk("  panic           - Trigger kernel panic test\n");
    } else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    } else if (strcmp(cmd, "cpuid") == 0 || strcmp(cmd, "cpuinfo") == 0) {
        cpuid_print_info();
    } else if (strcmp(cmd, "pci") == 0) {
        pci_list();
    } else if (strcmp(cmd, "disk") == 0) {
        ata_list_drives();
    } else if (strcmp(cmd, "date") == 0) {
        rtc_time_t t = rtc_read_time();
        printk("RTC Time: %04u-%02u-%02u %02u:%02u:%02u (Unix: %u)\n",
               t.year, t.month, t.day, t.hour, t.minute, t.second, rtc_get_ticks());
    } else if (strcmp(cmd, "dmesg") == 0 || strcmp(cmd, "klog") == 0) {
        klog_dump();
    } else if (strcmp(cmd, "lsmod") == 0) {
        module_list();
    } else if (strcmp(cmd, "smp") == 0) {
        smp_list_cpus();
    } else if (strcmp(cmd, "vesa") == 0) {
        vesa_test_demo();
    } else if (strcmp(cmd, "memstat") == 0) {
        debug_mem_stats();
    } else if (strcmp(cmd, "backtrace") == 0) {
        debug_dump_stack(8);
    } else if (strcmp(cmd, "hexdump") == 0) {
        if (arg1) {
            uint32_t addr = atox(arg1);
            uint32_t len = arg2 ? atoi(arg2) : 64;
            debug_hexdump((const void*)addr, len);
        } else {
            printk("Usage: hexdump <hex_address> [length]\n");
        }
    } else if (strcmp(cmd, "ps") == 0) {
        scheduler_list();
    } else if (strcmp(cmd, "ls") == 0) {
        vfs_list(arg1 ? arg1 : "/ram");
    } else if (strcmp(cmd, "cat") == 0) {
        if (arg1) {
            char path_buf[256];
            if (arg1[0] == '/') {
                strcpy(path_buf, arg1);
            } else {
                strcpy(path_buf, "/ram/");
                strcat(path_buf, arg1);
            }
            vfs_node_t *node = vfs_open(path_buf);
            if (node) {
                uint8_t read_buf[1025];
                int bytes = vfs_read(node, 0, 1024, read_buf);
                if (bytes >= 0) {
                    read_buf[bytes] = '\0';
                    printk("%s\n", (char*)read_buf);
                } else {
                    printk("Error reading file: %s\n", path_buf);
                }
                vfs_close(node);
            } else {
                printk("File not found: %s\n", path_buf);
            }
        } else {
            printk("Usage: cat <filename>\n");
        }
    } else if (strcmp(cmd, "echo") == 0) {
        int redirect_pos = -1;
        for (int i = 0; orig_copy[i]; i++) {
            if (orig_copy[i] == '>') {
                redirect_pos = i;
                break;
            }
        }

        if (redirect_pos != -1) {
            orig_copy[redirect_pos] = '\0';
            char *msg = orig_copy + 4; // Skip "echo"
            while (*msg == ' ') msg++;

            char *filename = orig_copy + redirect_pos + 1;
            while (*filename == ' ') filename++;

            uint32_t fname_len = strlen(filename);
            while (fname_len > 0 && filename[fname_len - 1] == ' ') {
                filename[fname_len - 1] = '\0';
                fname_len--;
            }

            ramfs_create(filename);
            ramfs_write(filename, (const uint8_t*)msg, strlen(msg));
            printk("Written to file '%s'\n", filename);
        } else {
            if (arg1) {
                printk("%s\n", arg1);
            } else {
                printk("\n");
            }
        }
    } else if (strcmp(cmd, "uptime") == 0) {
        uint32_t secs = timer_get_ticks() / 1000;
        printk("System Uptime: %u seconds\n", secs);
    } else if (strcmp(cmd, "reboot") == 0) {
        acpi_reboot();
    } else if (strcmp(cmd, "shutdown") == 0) {
        acpi_shutdown();
    } else if (strcmp(cmd, "panic") == 0) {
        kernel_panic("User triggered panic test from shell");
    } else {
        printk("Unknown command: '%s'. Type 'help' for available commands.\n", cmd);
    }
}

void shell_run(void) {
    shell_init();
    shell_prompt();
    cmd_pos = 0;

    for (;;) {
        char c = keyboard_read();

        if (c == '\n') {
            vga_putchar('\n');
            cmd_buf[cmd_pos] = '\0';
            shell_exec(cmd_buf);
            cmd_pos = 0;
            shell_prompt();
        } else if (c == '\b') {
            if (cmd_pos > 0) {
                cmd_pos--;
                vga_putchar('\b');
            }
        } else if (c >= 32 && c < 127) {
            if (cmd_pos < 255) {
                cmd_buf[cmd_pos++] = c;
                vga_putchar(c);
            }
        }
    }
}
