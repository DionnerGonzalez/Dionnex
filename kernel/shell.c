/* ============================================================================
 * Dionnex Kernel - Kernel Interactive Shell (kernel/shell.c)
 * ============================================================================
 */
#include <kernel/kernel.h>
#include <kernel/kabi.h>

static char input_buf[128];
static int buf_idx = 0;

void process_shell_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        kprintf("Dionnex Kernel Shell (dionnex-sh v1.0.0)\n");
        kprintf("Available commands:\n");
        kprintf("  help     - Show command overview\n");
        kprintf("  version  - Display kernel release information\n");
        kprintf("  mem      - Print physical & virtual memory usage\n");
        kprintf("  ls       - List virtual file system contents\n");
        kprintf("  cat <f>  - Read content of a file in VFS\n");
        kprintf("  ps       - List running process control blocks\n");
        kprintf("  ticks    - Show current timer ticks since boot\n");
        kprintf("  clear    - Clear screen buffer\n");
        kprintf("  panic    - Trigger a kernel panic test\n");
    } else if (strcmp(cmd, "version") == 0) {
        kprintf("Dionnex Modern Architecture Kernel v1.0.0-release (i686/x86_64)\n");
        kprintf("Compiled for Dionnex OS Execution Engine\n");
    } else if (strcmp(cmd, "mem") == 0) {
        uint32_t used = pmm_get_used_pages();
        uint32_t total = pmm_get_total_pages();
        kprintf("Memory Statistics:\n");
        kprintf("  Total Physical Pages : %u (%u MB)\n", total, (total * 4096) / (1024 * 1024));
        kprintf("  Used Physical Pages  : %u (%u KB)\n", used, (used * 4096) / 1024);
        kprintf("  Free Physical Pages  : %u (%u KB)\n", total - used, ((total - used) * 4096) / 1024);
    } else if (strcmp(cmd, "ls") == 0) {
        vfs_ls();
    } else if (strncmp(cmd, "cat ", 4) == 0) {
        const char* fname = cmd + 4;
        const char* content = vfs_read_file(fname);
        if (content) {
            kprintf("--- %s ---\n%s\n", fname, content);
        } else {
            kprintf("cat: file '%s' not found in VFS\n", fname);
        }
    } else if (strcmp(cmd, "ps") == 0) {
        kprintf("PID   NAME        STATE      PRIORITY\n");
        kprintf("0     idle        RUNNING    0\n");
        kprintf("1     init        READY      1\n");
        kprintf("2     dionnex-sh  RUNNING    2\n");
    } else if (strcmp(cmd, "ticks") == 0) {
        kprintf("Timer ticks elapsed since kernel boot: %u\n", (uint32_t)pit_get_ticks());
    } else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    } else if (strcmp(cmd, "panic") == 0) {
        panic("User requested test kernel panic from shell.");
    } else if (strlen(cmd) > 0) {
        kprintf("dionnex-sh: command not found: %s. Type 'help' for available commands.\n", cmd);
    }
}

void shell_start(void) {
    vga_set_color(0x0A, 0x00); // Light green prompt
    kprintf("dionnex-sh# ");
    vga_set_color(0x0F, 0x00);

    buf_idx = 0;
    memset(input_buf, 0, sizeof(input_buf));

    while (1) {
        char c = keyboard_read_char();
        if (c != 0) {
            if (c == '\n') {
                kprintf("\n");
                input_buf[buf_idx] = '\0';
                process_shell_command(input_buf);
                
                buf_idx = 0;
                memset(input_buf, 0, sizeof(input_buf));

                vga_set_color(0x0A, 0x00);
                kprintf("dionnex-sh# ");
                vga_set_color(0x0F, 0x00);
            } else if (c == '\b') {
                if (buf_idx > 0) {
                    buf_idx--;
                    input_buf[buf_idx] = '\0';
                    vga_putc('\b');
                }
            } else if (buf_idx < (int)sizeof(input_buf) - 1) {
                input_buf[buf_idx++] = c;
                vga_putc(c);
            }
        }
    }
}
