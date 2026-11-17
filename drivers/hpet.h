/* ============================================================================
 * Dionnex Monolithic Kernel - High Precision Event Timer (HPET) Header
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_HPET_H_
#define _DIONNEX_HPET_H_

#include <types.h>

#define HPET_REG_CAP_ID     0x000U
#define HPET_REG_GEN_CONF   0x010U
#define HPET_REG_INT_STATUS 0x020U
#define HPET_REG_MAIN_CNT   0x0F0U

void hpet_init(void);
int hpet_is_available(void);
uint64_t hpet_read_counter(void);
uint32_t hpet_get_freq_hz(void);
void hpet_delay_us(uint32_t microseconds);

#endif /* _DIONNEX_HPET_H_ */
