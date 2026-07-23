/* ============================================================================
 * Dionnex Monolithic Kernel - Real Time Clock (RTC) Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_RTC_H_
#define _DIONNEX_RTC_H_

#include <types.h>

struct rtc_time {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

typedef struct rtc_time rtc_time_t;

void rtc_init(void);
rtc_time_t rtc_read_time(void);
uint32_t rtc_get_ticks(void);

#endif /* _DIONNEX_RTC_H_ */
