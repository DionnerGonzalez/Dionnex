/* ============================================================================
 * Dionnex Monolithic Kernel - VGA Display Driver Implementation
 * ============================================================================
 */
#include <drivers/vga.h>
#include <io.h>

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

static uint16_t *vga_buffer = (uint16_t*)VGA_BUFFER;
static uint8_t vga_row = 0;
static uint8_t vga_col = 0;
static uint8_t vga_color = 0x0F;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

static void vga_memmove(uint16_t *dst, uint16_t *src, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        dst[i] = src[i];
    }
}

void vga_update_cursor(void) {
    uint16_t pos = (uint16_t)(vga_row * VGA_WIDTH + vga_col);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void vga_scroll(void) {
    vga_memmove(vga_buffer, vga_buffer + VGA_WIDTH, VGA_WIDTH * (VGA_HEIGHT - 1));
    for (uint32_t i = 0; i < VGA_WIDTH; i++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = vga_entry(' ', vga_color);
    }
    vga_row = VGA_HEIGHT - 1;
}

void vga_clear(void) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = vga_entry(' ', vga_color);
    }
    vga_row = 0;
    vga_col = 0;
    vga_update_cursor();
}

void vga_init(void) {
    vga_color = 0x0F;
    vga_clear();
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(' ', vga_color);
        }
    } else {
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_color);
        vga_col++;
    }

    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }

    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
    }

    vga_update_cursor();
}

void vga_write(const char *s) {
    if (!s) return;
    while (*s) {
        vga_putchar(*s++);
    }
}

void vga_write_hex(uint32_t val) {
    vga_write("0x");
    static const char hex_chars[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (val >> i) & 0x0F;
        vga_putchar(hex_chars[nibble]);
    }
}

void vga_write_dec(uint32_t val) {
    if (val == 0) {
        vga_putchar('0');
        return;
    }
    char buf[12];
    int pos = 0;
    while (val > 0) {
        buf[pos++] = (char)('0' + (val % 10));
        val /= 10;
    }
    for (int i = pos - 1; i >= 0; i--) {
        vga_putchar(buf[i]);
    }
}
