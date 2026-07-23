/* ============================================================================
 * Dionnex Monolithic Kernel - Interactive Kernel Shell Implementation
 * ============================================================================
 */
#include <kernel/shell.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/timer.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <mm/pmm.h>
#include <sched/scheduler.h>
#include <fs/ramfs.h>
#include <io.h>

static char cmd_buf[256];
static uint32_t cmd_pos = 0;

static int shell_strcmp(const char *a, const char *b) {
    if (!a || !b) return -1;
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

static uint32_t shell_strlen(const char *s) {
    uint32_t len = 0;
    while (s && s[len]) len++;
    return len;
}

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
    printk("  ======================================\n");
    printk("  Dionnex Monolithic Kernel v0.1\n");
    printk("  x86 32-bit | GPLv2\n");
    printk("  ======================================\n");
    printk("\n");
    printk("  Type 'help' for available commands.\n\n");
}

static void shell_exec(char *buf) {
    char *cmd = NULL;
    char *arg1 = NULL;
    char *arg2 = NULL;

    char orig_copy[256];
    uint32_t len = shell_strlen(buf);
    for (uint32_t i = 0; i <= len; i++) {
        orig_copy[i] = buf[i];
    }

    shell_parse(buf, &cmd, &arg1, &arg2);

    if (!cmd || shell_strlen(cmd) == 0) return;

    if (shell_strcmp(cmd, "help") == 0) {
        printk("Dionnex Kernel Shell Commands:\n");
        printk("  help       - Show available commands\n");
        printk("  clear      - Clear VGA screen\n");
        printk("  meminfo    - Show memory allocation details\n");
        printk("  ps         - List active scheduler tasks\n");
        printk("  ls         - List files in RAMFS\n");
        printk("  touch <f>  - Create a new file\n");
        printk("  cat <f>    - Display contents of a file\n");
        printk("  echo <msg> - Print message or echo <msg> > <file>\n");
        printk("  uptime     - Show system uptime in seconds\n");
        printk("  reboot     - Reboot system hardware\n");
        printk("  shutdown   - Power off QEMU emulator\n");
        printk("  panic      - Trigger kernel panic test\n");
    } else if (shell_strcmp(cmd, "clear") == 0) {
        vga_clear();
    } else if (shell_strcmp(cmd, "meminfo") == 0) {
        uint32_t total = pmm_get_total_count();
        uint32_t free = pmm_get_free_count();
        printk("Memory Info:\n");
        printk("  Total Memory: %u KB\n", total * 4);
        printk("  Free Memory:  %u KB\n", free * 4);
        printk("  Used Memory:  %u KB\n", (total - free) * 4);
    } else if (shell_strcmp(cmd, "ps") == 0) {
        scheduler_list();
    } else if (shell_strcmp(cmd, "ls") == 0) {
        ramfs_list();
    } else if (shell_strcmp(cmd, "touch") == 0) {
        if (arg1) {
            if (ramfs_create(arg1) == 0) {
                printk("Created file '%s'\n", arg1);
            } else {
                printk("Error: Could not create file '%s'\n", arg1);
            }
        } else {
            printk("Usage: touch <filename>\n");
        }
    } else if (shell_strcmp(cmd, "cat") == 0) {
        if (arg1) {
            uint8_t read_buf[1025];
            int bytes = ramfs_read(arg1, read_buf, 1024);
            if (bytes >= 0) {
                read_buf[bytes] = '\0';
                printk("%s\n", (char*)read_buf);
            } else {
                printk("File not found: %s\n", arg1);
            }
        } else {
            printk("Usage: cat <filename>\n");
        }
    } else if (shell_strcmp(cmd, "echo") == 0) {
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

            uint32_t fname_len = shell_strlen(filename);
            while (fname_len > 0 && filename[fname_len - 1] == ' ') {
                filename[fname_len - 1] = '\0';
                fname_len--;
            }

            ramfs_create(filename);
            ramfs_write(filename, (const uint8_t*)msg, shell_strlen(msg));
            printk("Written to file '%s'\n", filename);
        } else {
            if (arg1) {
                printk("%s\n", arg1);
            } else {
                printk("\n");
            }
        }
    } else if (shell_strcmp(cmd, "uptime") == 0) {
        uint32_t secs = timer_get_ticks() / 1000;
        printk("System Uptime: %u seconds\n", secs);
    } else if (shell_strcmp(cmd, "reboot") == 0) {
        printk("Rebooting Dionnex Kernel...\n");
        outb(0x64, 0xFE);
    } else if (shell_strcmp(cmd, "shutdown") == 0) {
        printk("Shutting down system...\n");
        outw(0x604, 0x2000);
    } else if (shell_strcmp(cmd, "panic") == 0) {
        kernel_panic("User triggered panic from shell");
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
