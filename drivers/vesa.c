/* ============================================================================
 * Dionnex Monolithic Kernel - VESA VBE Framebuffer Driver Implementation
 * ============================================================================
 */
#include <drivers/vesa.h>
#include <kernel/printk.h>
#include <kernel/multiboot.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <string.h>

static uint32_t vesa_width = 800;
static uint32_t vesa_height = 600;
static uint32_t vesa_bpp = 32;
static uint32_t vesa_pitch = 3200;
static uint8_t *vesa_framebuffer = NULL;
static int vesa_enabled = 0;

// Embedded 8x8 basic ASCII font (0x20 to 0x7E)
static const uint8_t font8x8_basic[128][8] = {
    ['A'] = {0x18, 0x3C, 0x66, 0x76, 0x7E, 0x66, 0x66, 0x00},
    ['B'] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    ['C'] = {0x3C, 0x66, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['D'] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    ['E'] = {0x7E, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x7E, 0x00},
    ['F'] = {0x7E, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x06, 0x00},
    ['G'] = {0x3C, 0x66, 0x06, 0x76, 0x66, 0x66, 0x3C, 0x00},
    ['H'] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['I'] = {0x3E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3E, 0x00},
    ['J'] = {0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00},
    ['K'] = {0x66, 0x6C, 0x78, 0x30, 0x78, 0x6C, 0x66, 0x00},
    ['L'] = {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7E, 0x00},
    ['M'] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},
    ['N'] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    ['O'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['P'] = {0x7C, 0x66, 0x66, 0x7C, 0x06, 0x06, 0x06, 0x00},
    ['Q'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x50, 0x00},
    ['R'] = {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00},
    ['S'] = {0x3C, 0x66, 0x06, 0x3C, 0x60, 0x66, 0x3C, 0x00},
    ['T'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['V'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['W'] = {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
    ['X'] = {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},
    ['Y'] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
    ['Z'] = {0x7E, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x7E, 0x00},
    ['0'] = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
    ['1'] = {0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E, 0x00},
    ['2'] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x06, 0x7E, 0x00},
    ['3'] = {0x3C, 0x66, 0x60, 0x3C, 0x60, 0x66, 0x3C, 0x00},
    ['4'] = {0x60, 0x66, 0x66, 0x7E, 0x60, 0x60, 0x60, 0x00},
    ['5'] = {0x7E, 0x06, 0x3C, 0x60, 0x60, 0x66, 0x3C, 0x00},
    ['6'] = {0x3C, 0x66, 0x06, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    ['7'] = {0x7E, 0x60, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},
    ['8'] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    ['9'] = {0x3C, 0x66, 0x66, 0x7C, 0x60, 0x66, 0x3C, 0x00},
    [':'] = {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00},
    ['-'] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    ['!'] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00},
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

void vesa_init(void *mb_info_ptr) {
    if (!mb_info_ptr) {
        printk("VESA: Not available (VGA text mode fallback)\n");
        vesa_enabled = 0;
        return;
    }

    multiboot_info_t *mb = (multiboot_info_t*)mb_info_ptr;

    // Check if VBE mode info is present (bit 11)
    if ((mb->flags & (1 << 11)) && mb->vbe_mode_info != 0) {
        vesa_mode_info_t *mode_info = (vesa_mode_info_t*)mb->vbe_mode_info;
        vesa_width  = mode_info->width;
        vesa_height = mode_info->height;
        vesa_bpp    = mode_info->bpp;
        vesa_pitch  = mode_info->bytes_per_scanline;
        vesa_framebuffer = (uint8_t*)mode_info->physbase;

        // Map framebuffer pages
        uint32_t fb_size = vesa_pitch * vesa_height;
        uint32_t fb_pages = (fb_size + PAGE_SIZE - 1) / PAGE_SIZE;
        for (uint32_t i = 0; i < fb_pages; i++) {
            vmm_map_page((uint32_t)vesa_framebuffer + (i * PAGE_SIZE),
                         (uint32_t)vesa_framebuffer + (i * PAGE_SIZE),
                         VMM_PRESENT | VMM_WRITABLE);
        }

        vesa_enabled = 1;
        printk("VESA: %ux%u %ubpp framebuffer at 0x%x\n",
               vesa_width, vesa_height, vesa_bpp, (uint32_t)vesa_framebuffer);
    } else {
        // Fallback: Allocate software canvas buffer
        uint32_t size = vesa_width * vesa_height * 4;
        vesa_framebuffer = (uint8_t*)kmalloc(size);
        if (vesa_framebuffer) {
            memset(vesa_framebuffer, 0, size);
            vesa_enabled = 1;
            printk("VESA: Software Canvas %ux%u 32bpp allocated at 0x%x\n",
                   vesa_width, vesa_height, (uint32_t)vesa_framebuffer);
        } else {
            vesa_enabled = 0;
            printk("VESA: Not available (VGA text mode fallback)\n");
        }
    }
}

int vesa_is_enabled(void) { return vesa_enabled; }
uint32_t vesa_get_width(void) { return vesa_width; }
uint32_t vesa_get_height(void) { return vesa_height; }
uint32_t vesa_get_bpp(void) { return vesa_bpp; }

void vesa_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!vesa_enabled || !vesa_framebuffer) return;
    if (x >= vesa_width || y >= vesa_height) return;

    if (vesa_bpp == 32) {
        uint32_t *fb = (uint32_t*)(vesa_framebuffer + (y * vesa_pitch));
        fb[x] = color;
    } else if (vesa_bpp == 24) {
        uint8_t *fb = vesa_framebuffer + (y * vesa_pitch) + (x * 3);
        fb[0] = color & 0xFF;         // Blue
        fb[1] = (color >> 8) & 0xFF;  // Green
        fb[2] = (color >> 16) & 0xFF; // Red
    }
}

uint32_t vesa_getpixel(uint32_t x, uint32_t y) {
    if (!vesa_enabled || !vesa_framebuffer) return 0;
    if (x >= vesa_width || y >= vesa_height) return 0;

    if (vesa_bpp == 32) {
        uint32_t *fb = (uint32_t*)(vesa_framebuffer + (y * vesa_pitch));
        return fb[x];
    }
    return 0;
}

void vesa_clear(uint32_t color) {
    if (!vesa_enabled) return;
    for (uint32_t y = 0; y < vesa_height; y++) {
        for (uint32_t x = 0; x < vesa_width; x++) {
            vesa_putpixel(x, y, color);
        }
    }
}

void vesa_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            vesa_putpixel(x + j, y + i, color);
        }
    }
}

void vesa_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t i = 0; i < w; i++) {
        vesa_putpixel(x + i, y, color);
        vesa_putpixel(x + i, y + h - 1, color);
    }
    for (uint32_t i = 0; i < h; i++) {
        vesa_putpixel(x, y + i, color);
        vesa_putpixel(x + w - 1, y + i, color);
    }
}

void vesa_draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        vesa_putpixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

void vesa_draw_circle(int xc, int yc, int r, uint32_t color) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        vesa_putpixel(xc + x, yc + y, color);
        vesa_putpixel(xc - x, yc + y, color);
        vesa_putpixel(xc + x, yc - y, color);
        vesa_putpixel(xc - x, yc - y, color);
        vesa_putpixel(xc + y, yc + x, color);
        vesa_putpixel(xc - y, yc + x, color);
        vesa_putpixel(xc + y, yc - x, color);
        vesa_putpixel(xc - y, yc - x, color);
        x++;
        if (d > 0) { y--; d = d + 4 * (x - y) + 10; }
        else d = d + 4 * x + 6;
    }
}

void vesa_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    uint8_t uc = (uint8_t)c;
    const uint8_t *glyph = font8x8_basic[uc];

    for (int i = 0; i < 8; i++) {
        uint8_t row = glyph[i];
        for (int j = 0; j < 8; j++) {
            if (row & (1 << (7 - j))) {
                vesa_putpixel(x + j, y + i, fg);
            } else if (bg != 0xFFFFFFFF) { // Transparency check
                vesa_putpixel(x + j, y + i, bg);
            }
        }
    }
}

void vesa_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg) {
    if (!str) return;
    uint32_t cur_x = x;
    while (*str) {
        vesa_draw_char(cur_x, y, *str, fg, bg);
        cur_x += 8;
        str++;
    }
}

void vesa_test_demo(void) {
    if (!vesa_enabled) {
        printk("VESA: Canvas not enabled\n");
        return;
    }

    // Dark Blue background
    vesa_clear(0x000B1E);

    // Color gradient bar
    for (uint32_t x = 50; x < 750; x++) {
        uint32_t red   = (x * 255) / 750;
        uint32_t green = 128;
        uint32_t blue  = 255 - red;
        uint32_t color = (red << 16) | (green << 8) | blue;
        for (uint32_t y = 50; y < 80; y++) {
            vesa_putpixel(x, y, color);
        }
    }

    // Draw shapes
    vesa_fill_rect(100, 150, 120, 80, 0x00FF88);
    vesa_draw_rect(250, 150, 120, 80, 0xFFCC00);
    vesa_draw_circle(450, 190, 40, 0xFF0055);
    vesa_draw_line(550, 150, 700, 230, 0x00FFFF);

    // Text output
    vesa_draw_string(100, 300, "DIONNEX MONOLITHIC KERNEL VESA DEMO", 0xFFFFFF, 0xFFFFFFFF);
    vesa_draw_string(100, 320, "FRAMEBUFFER GRAPHICS TEST OK!", 0x00FF00, 0xFFFFFFFF);

    printk("VESA: Test graphics rendered to framebuffer\n");
}
