/* ============================================================================
 * Dionnex Kernel - Formatted Kernel Printf & VGA Driver (kernel/kprintf.c)
 * ============================================================================
 */
#include <kernel/kernel.h>
#include <stdarg.h>

static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static uint8_t vga_color = 0x0F; // White on black
static size_t vga_col = 0;
static size_t vga_row = 0;

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = (bg << 4) | (fg & 0x0F);
}

void vga_clear(void) {
    for (size_t y = 0; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            vga_buffer[y * 80 + x] = (vga_color << 8) | ' ';
        }
    }
    vga_col = 0;
    vga_row = 0;
}

static void vga_scroll(void) {
    for (size_t y = 0; y < 24; y++) {
        for (size_t x = 0; x < 80; x++) {
            vga_buffer[y * 80 + x] = vga_buffer[(y + 1) * 80 + x];
        }
    }
    for (size_t x = 0; x < 80; x++) {
        vga_buffer[24 * 80 + x] = (vga_color << 8) | ' ';
    }
    vga_row = 24;
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            vga_buffer[vga_row * 80 + vga_col] = (vga_color << 8) | ' ';
        }
    } else if (c == '\t') {
        vga_col = (vga_col + 4) & ~3;
    } else {
        vga_buffer[vga_row * 80 + vga_col] = (vga_color << 8) | (uint8_t)c;
        vga_col++;
    }

    if (vga_col >= 80) {
        vga_col = 0;
        vga_row++;
    }

    if (vga_row >= 25) {
        vga_scroll();
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}

static void itoa(int val, char* buf, int base) {
    static char num[] = "0123456789abcdef";
    char* wstr = buf;
    int sign = 0;

    if (val < 0 && base == 10) {
        sign = 1;
        val = -val;
    }

    unsigned int uval = (unsigned int)val;

    do {
        *wstr++ = num[uval % base];
    } while (uval /= base);

    if (sign) *wstr++ = '-';
    *wstr = '\0';

    // Reverse
    char* start = buf;
    char* end = wstr - 1;
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

static void utoa(uint32_t val, char* buf, int base) {
    static char num[] = "0123456789ABCDEF";
    char* wstr = buf;

    do {
        *wstr++ = num[val % base];
    } while (val /= base);

    *wstr = '\0';

    // Reverse
    char* start = buf;
    char* end = wstr - 1;
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buf[32];

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    char* s = va_arg(args, char*);
                    if (!s) s = "(null)";
                    vga_puts(s);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    vga_putc(c);
                    break;
                }
                case 'd':
                case 'i': {
                    int d = va_arg(args, int);
                    itoa(d, buf, 10);
                    vga_puts(buf);
                    break;
                }
                case 'u': {
                    uint32_t u = va_arg(args, uint32_t);
                    utoa(u, buf, 10);
                    vga_puts(buf);
                    break;
                }
                case 'x':
                case 'p': {
                    uint32_t x = va_arg(args, uint32_t);
                    vga_puts("0x");
                    utoa(x, buf, 16);
                    vga_puts(buf);
                    break;
                }
                case '%': {
                    vga_putc('%');
                    break;
                }
                default:
                    vga_putc('%');
                    vga_putc(*fmt);
                    break;
            }
        } else {
            vga_putc(*fmt);
        }
        fmt++;
    }

    va_end(args);
}
