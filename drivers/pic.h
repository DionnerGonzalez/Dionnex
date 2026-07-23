/* ============================================================================
 * Dionnex Monolithic Kernel - Programmable Interrupt Controller (PIC) Header
 * ============================================================================
 */
#ifndef _DIONNEX_PIC_H_
#define _DIONNEX_PIC_H_

#include <types.h>

void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void pic_disable(void);
void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);

#endif /* _DIONNEX_PIC_H_ */
