/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Formatted Output Implementation
 * ============================================================================
 */
#include <kernel/printk.h>
#include <drivers/vga.h>
#include <drivers/serial.h>
#include <string.h>
#include <stdarg.h>

static void printk_utoa(uint32_t val, char *buf, int base, bool uppercase) {
    static const char digits_lower[] = "0123456789abcdef";
    static const char digits_upper[] = "0123456789ABCDEF";
    const char *digits = uppercase ? digits_upper : digits_lower;

    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char tmp[32];
    int pos = 0;
    while (val > 0) {
        tmp[pos++] = digits[val % base];
        val /= base;
    }

    int i = 0;
    for (int j = pos - 1; j >= 0; j--) {
        buf[i++] = tmp[j];
    }
    buf[i] = '\0';
}

static void printk_itoa(int32_t val, char *buf, int base) {
    if (val < 0 && base == 10) {
        buf[0] = '-';
        printk_utoa((uint32_t)(-val), buf + 1, base, false);
    } else {
        printk_utoa((uint32_t)val, buf, base, false);
    }
}

static void printk_out_char(char c) {
    vga_putchar(c);
    if (c == '\n') serial_putchar('\r');
    serial_putchar(c);
}

static void printk_out_str(const char *s) {
    if (!s) return;
    while (*s) {
        printk_out_char(*s++);
    }
}

void printk(const char *fmt, ...) {
    if (!fmt) return;

    va_list args;
    va_start(args, fmt);

    char buf[64];

    while (*fmt) {
        if (*fmt != '%') {
            printk_out_char(*fmt);
            fmt++;
            continue;
        }

        fmt++; // Skip '%'
        if (!*fmt) break;

        switch (*fmt) {
            case 's': {
                const char *str = va_arg(args, const char *);
                if (!str) str = "(null)";
                printk_out_str(str);
                break;
            }
            case 'd':
            case 'i': {
                int32_t val = va_arg(args, int32_t);
                printk_itoa(val, buf, 10);
                printk_out_str(buf);
                break;
            }
            case 'u': {
                uint32_t val = va_arg(args, uint32_t);
                printk_utoa(val, buf, 10, false);
                printk_out_str(buf);
                break;
            }
            case 'x': {
                uint32_t val = va_arg(args, uint32_t);
                printk_utoa(val, buf, 16, false);
                printk_out_str(buf);
                break;
            }
            case 'X': {
                uint32_t val = va_arg(args, uint32_t);
                printk_utoa(val, buf, 16, true);
                printk_out_str(buf);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                printk_out_char(c);
                break;
            }
            case '%': {
                printk_out_char('%');
                break;
            }
            default: {
                printk_out_char('%');
                printk_out_char(*fmt);
                break;
            }
        }
        fmt++;
    }

    va_end(args);
}
