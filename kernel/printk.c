/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Formatted Output Implementation
 * Author: Ihosvanni Dionner Gonzalez
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

static void printk_out_padded(const char *s, int width, int left_align, char pad_char) {
    if (!s) s = "(null)";
    int len = (int)strlen(s);
    int pad = (width > len) ? (width - len) : 0;

    if (!left_align) {
        if (pad_char == '0' && s[0] == '-') {
            printk_out_char('-');
            s++;
        }
        for (int i = 0; i < pad; i++) {
            printk_out_char(pad_char);
        }
    }

    while (*s) {
        printk_out_char(*s++);
    }

    if (left_align) {
        for (int i = 0; i < pad; i++) {
            printk_out_char(' ');
        }
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

        fmt++; /* Skip '%' */
        if (!*fmt) break;

        int left_align = 0;
        char pad_char = ' ';
        int width = 0;

        while (*fmt == '-' || *fmt == '0') {
            if (*fmt == '-') left_align = 1;
            else if (*fmt == '0') pad_char = '0';
            fmt++;
        }
        if (left_align) pad_char = ' ';

        while (*fmt >= '0' && *fmt <= '9') {
            width = (width * 10) + (*fmt - '0');
            fmt++;
        }

        if (*fmt == 'l') fmt++;

        switch (*fmt) {
            case 's': {
                const char *str = va_arg(args, const char *);
                printk_out_padded(str ? str : "(null)", width, left_align, ' ');
                break;
            }
            case 'd':
            case 'i': {
                int32_t val = va_arg(args, int32_t);
                printk_itoa(val, buf, 10);
                printk_out_padded(buf, width, left_align, pad_char);
                break;
            }
            case 'u': {
                uint32_t val = va_arg(args, uint32_t);
                printk_utoa(val, buf, 10, false);
                printk_out_padded(buf, width, left_align, pad_char);
                break;
            }
            case 'x': {
                uint32_t val = va_arg(args, uint32_t);
                printk_utoa(val, buf, 16, false);
                printk_out_padded(buf, width, left_align, pad_char);
                break;
            }
            case 'X': {
                uint32_t val = va_arg(args, uint32_t);
                printk_utoa(val, buf, 16, true);
                printk_out_padded(buf, width, left_align, pad_char);
                break;
            }
            case 'p': {
                uint32_t val = va_arg(args, uint32_t);
                printk_out_padded("0x", 0, 0, ' ');
                printk_utoa(val, buf, 16, false);
                printk_out_padded(buf, 8, 0, '0');
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                char ch_buf[2] = { c, '\0' };
                printk_out_padded(ch_buf, width, left_align, ' ');
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
