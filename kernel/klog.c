/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Logging Subsystem Implementation
 * ============================================================================
 */
#include <kernel/klog.h>
#include <kernel/timer.h>
#include <drivers/serial.h>
#include <drivers/vga.h>
#include <string.h>
#include <stdarg.h>

static char klog_buffer[DIONNEX_KLOG_SIZE];
static uint32_t klog_head = 0;
static uint32_t klog_tail = 0;
static uint32_t klog_count = 0;

static const char *level_strings[] = {
    "[DEBUG]",
    "[INFO] ",
    "[WARN] ",
    "[ERROR]",
    "[PANIC]"
};

static void klog_push_char(char c) {
    klog_buffer[klog_head] = c;
    klog_head = (klog_head + 1) % DIONNEX_KLOG_SIZE;
    if (klog_count < DIONNEX_KLOG_SIZE) {
        klog_count++;
    } else {
        klog_tail = (klog_tail + 1) % DIONNEX_KLOG_SIZE;
    }
}

static void klog_push_string(const char *s) {
    if (!s) return;
    while (*s) {
        klog_push_char(*s++);
    }
}

void klog_init(void) {
    klog_head = 0;
    klog_tail = 0;
    klog_count = 0;
    memset(klog_buffer, 0, DIONNEX_KLOG_SIZE);
}

void klog(uint8_t level, const char *fmt, ...) {
    if (level > KLOG_PANIC) level = KLOG_INFO;

    char msg_buf[512];
    char num_buf[32];
    uint32_t ticks = timer_get_ticks();

    // Format header: "[12345] [INFO] "
    msg_buf[0] = '\0';
    strcat(msg_buf, "[");
    utoa(ticks, num_buf, 10);
    strcat(msg_buf, num_buf);
    strcat(msg_buf, "] ");
    strcat(msg_buf, level_strings[level]);
    strcat(msg_buf, " ");

    uint32_t hdr_len = strlen(msg_buf);
    char *out_ptr = msg_buf + hdr_len;

    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                char *s = va_arg(args, char*);
                if (!s) s = "(null)";
                while (*s) *out_ptr++ = *s++;
            } else if (*fmt == 'd') {
                int d = va_arg(args, int);
                itoa(d, num_buf, 10);
                char *p = num_buf;
                while (*p) *out_ptr++ = *p++;
            } else if (*fmt == 'u') {
                uint32_t u = va_arg(args, uint32_t);
                utoa(u, num_buf, 10);
                char *p = num_buf;
                while (*p) *out_ptr++ = *p++;
            } else if (*fmt == 'x' || *fmt == 'X') {
                uint32_t x = va_arg(args, uint32_t);
                utoa(x, num_buf, 16);
                char *p = num_buf;
                while (*p) *out_ptr++ = *p++;
            } else if (*fmt == 'c') {
                char c = (char)va_arg(args, int);
                *out_ptr++ = c;
            } else if (*fmt == '%') {
                *out_ptr++ = '%';
            }
        } else {
            *out_ptr++ = *fmt;
        }
        fmt++;
    }
    va_end(args);
    *out_ptr = '\0';

    klog_push_string(msg_buf);
    serial_write(msg_buf);
}

void klog_dump(void) {
    uint32_t idx = klog_tail;
    for (uint32_t i = 0; i < klog_count; i++) {
        char c = klog_buffer[idx];
        vga_putchar(c);
        serial_putchar(c);
        idx = (idx + 1) % DIONNEX_KLOG_SIZE;
    }
}

void klog_clear(void) {
    klog_head = 0;
    klog_tail = 0;
    klog_count = 0;
    memset(klog_buffer, 0, DIONNEX_KLOG_SIZE);
}

uint32_t klog_get_count(void) {
    return klog_count;
}

int klog_read_bytes(char *buf, uint32_t max_len) {
    if (!buf || max_len == 0) return 0;
    uint32_t idx = klog_tail;
    uint32_t read_cnt = (klog_count < max_len) ? klog_count : max_len;
    for (uint32_t i = 0; i < read_cnt; i++) {
        buf[i] = klog_buffer[idx];
        idx = (idx + 1) % DIONNEX_KLOG_SIZE;
    }
    return (int)read_cnt;
}
