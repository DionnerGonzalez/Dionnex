/* ============================================================================
 * Dionnex Monolithic Kernel - Real Time Clock (RTC) Driver Implementation
 * ============================================================================
 */
#include <drivers/rtc.h>
#include <kernel/printk.h>
#include <io.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

static int cmos_is_updating(void) {
    outb(CMOS_ADDR, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

static uint8_t bcd_to_bin(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

rtc_time_t rtc_read_time(void) {
    rtc_time_t t, last_t;

    // Wait if CMOS is updating
    while (cmos_is_updating()) ;

    t.second = cmos_read(0x00);
    t.minute = cmos_read(0x02);
    t.hour   = cmos_read(0x04);
    t.day    = cmos_read(0x07);
    t.month  = cmos_read(0x08);
    t.year   = cmos_read(0x09);

    // Read twice to ensure consistent reading
    do {
        last_t = t;
        while (cmos_is_updating()) ;
        t.second = cmos_read(0x00);
        t.minute = cmos_read(0x02);
        t.hour   = cmos_read(0x04);
        t.day    = cmos_read(0x07);
        t.month  = cmos_read(0x08);
        t.year   = cmos_read(0x09);
    } while (t.second != last_t.second || t.minute != last_t.minute ||
             t.hour != last_t.hour     || t.day != last_t.day       ||
             t.month != last_t.month   || t.year != last_t.year);

    uint8_t reg_b = cmos_read(0x0B);

    // Convert BCD to binary if needed
    if (!(reg_b & 0x04)) {
        t.second = bcd_to_bin(t.second);
        t.minute = bcd_to_bin(t.minute);
        t.hour   = bcd_to_bin(t.hour & 0x7F) | (t.hour & 0x80);
        t.day    = bcd_to_bin(t.day);
        t.month  = bcd_to_bin(t.month);
        t.year   = bcd_to_bin((uint8_t)t.year);
    }

    // Convert 12 hour to 24 hour format if needed
    if (!(reg_b & 0x02) && (t.hour & 0x80)) {
        t.hour = ((t.hour & 0x7F) + 12) % 24;
    }

    t.year += 2000;

    return t;
}

uint32_t rtc_get_ticks(void) {
    rtc_time_t t = rtc_read_time();

    uint16_t y = t.year;
    uint8_t m = t.month;
    uint8_t d = t.day;

    // Days since Jan 1, 1970 calculation
    if (m <= 2) {
        y -= 1;
        m += 12;
    }

    uint32_t era = y / 400;
    uint32_t yoe = y - era * 400;
    uint32_t doy = (153 * (m - 3) + 2) / 5 + d - 1;
    uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    uint32_t days = era * 146097 + doe - 719468;

    return days * 86400 + t.hour * 3600 + t.minute * 60 + t.second;
}

void rtc_init(void) {
    rtc_time_t t = rtc_read_time();
    printk("RTC: %04u-%02u-%02u %02u:%02u:%02u\n",
           t.year, t.month, t.day, t.hour, t.minute, t.second);
}
