/* ============================================================================
 * Dionnex Monolithic Kernel - PIT Timer Header
 * ============================================================================
 */
#ifndef _DIONNEX_TIMER_H_
#define _DIONNEX_TIMER_H_

#include <types.h>

void timer_init(uint32_t frequency);
uint32_t timer_get_ticks(void);
void timer_sleep(uint32_t ms);

#endif /* _DIONNEX_TIMER_H_ */
